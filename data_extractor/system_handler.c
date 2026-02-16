/*
 * system_init.c
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#include "system_handler.h"

#include "sensors_types.h"
#include "system_types.h"

#include "energy.h"
#include "vars_read.h"
#include "streaming_task.h"
#include "cmd_task.h"
#include "fw_update.h"
#include "time_sync.h"
#include "led_handler.h"

#include "ade9000_regs.h"

#include "watchdog.h"
#include "events.h"
#include "event_handler.h"
#include "manufacturing.h"
#include "mqtt_protocol.h"
#include "json.h"

#include "absl_system.h"
#include "absl_debug.h"
#include "absl_thread.h"
#include "absl_macros.h"
#include "absl_nvm.h"
#include "absl_gpio.h"
#include "absl_system.h"

#include "fw_version.h"
#include "absl_hw_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ENERGY_TASK_PRIORITY 		11
#define ENERGY_TASK_STACK_SIZE 		1024

#define FAST_READ_TASK_PRIORITY 	10
#define FAST_READ_TASK_STACK_SIZE 	900

#define SLOW_READ_TASK_PRIORITY 	9
#define SLOW_READ_TASK_STACK_SIZE 	800

#define FAST_STREAM_TASK_PRIORITY 	8
#define FAST_STREAM_TASK_STACK_SIZE 1024

#define SLOW_STREAM_TASK_PRIORITY 	6
#define SLOW_STREAM_TASK_STACK_SIZE 1024

#define CMD_TASK_PRIORITY 			5
#define CMD_TASK_STACK_SIZE 		4096

#define FW_UPDATE_TASK_PRIORITY 	4
#define FW_UPDATE_TASK_STACK_SIZE 	3072

#define TIMESYNC_PRIORITY 			12
#define TIMESYNC_STACK_SIZE 		500

#define LED_HANDLER_PRIORITY 		13
#define LED_HANDLER_STACK_SIZE 		500

#define WATCHDOG_PRIORITY 			14
#define WATCHDOG_STACK_SIZE 		500

#define PHY_AMOUNT					1

#define MAX_EVENT_NO_CONN			10

#define ERROR_RESET_TIME_S			60

#define I_MASK  0x01
#define V_MASK  0x02
#define S_MASK  0x04
#define P_MASK  0x08
#define Q_MASK  0x10

#define SYSTEM_CONNECTED_TO_SERVER			0x00000001
#define SYSTEM_CONFIGURED					0x00000002
#define SYSTEM_TO_FW_UPDATE					0x00000004
#define SYSTEM_EVENT_TO_PROCESS				0x00000008
#define SYSTEM_LINK_LOST					0x00000010
#define SYSTEM_NO_SERVER					0x00000020
#define SYSTEM_CONNECTION_TO_SERVER_LOST	0x00000040
#define SYSTEM_ADE9000_RECONFIG				0x00000080
#define SYSTEM_RAW_STATE_RUNNING			0x00000100
#define SYSTEM_RAW_STATE_STOPPED			0x00000200
#define SYSTEM_REG_STATE_RUNNING			0x00000400
#define SYSTEM_REG_STATE_STOPPED			0x00000800
#define SYSTEM_TIME_SYNC_FINISHED			0x00001000
#define SYSTEM_FW_AREA_ERASED				0x00002000
#define SYSTEM_FW_ERASE_NEEDED				0x00004000
#define SYSTEM_WATCHDOG						0x00008000

#define SYSTEM_WAIT_SERVER_STATE_EVENTS 			SYSTEM_CONNECTED_TO_SERVER 			| \
													SYSTEM_LINK_LOST		   			| \
													SYSTEM_NO_SERVER					| \
													SYSTEM_EVENT_TO_PROCESS				| \
													SYSTEM_WATCHDOG

#define SYSTEM_NORMAL_STATE_EVENTS					SYSTEM_CONFIGURED					| \
													SYSTEM_LINK_LOST					| \
													SYSTEM_CONNECTION_TO_SERVER_LOST	| \
													SYSTEM_TO_FW_UPDATE					| \
													SYSTEM_ADE9000_RECONFIG				| \
													SYSTEM_EVENT_TO_PROCESS				| \
													SYSTEM_RAW_STATE_RUNNING			| \
													SYSTEM_RAW_STATE_STOPPED			| \
													SYSTEM_REG_STATE_RUNNING			| \
													SYSTEM_REG_STATE_STOPPED			| \
													SYSTEM_WATCHDOG

#define SYSTEM_ERROR_STATE_EVENTS					SYSTEM_LINK_LOST					| \
													SYSTEM_CONNECTION_TO_SERVER_LOST	| \
													SYSTEM_TO_FW_UPDATE					| \
													SYSTEM_EVENT_TO_PROCESS				| \
													SYSTEM_WATCHDOG

#define SYSTEM_FW_UPDATE_EVENTS						SYSTEM_LINK_LOST					| \
													SYSTEM_CONNECTION_TO_SERVER_LOST	| \
													SYSTEM_EVENT_TO_PROCESS				| \
													SYSTEM_FW_ERASE_NEEDED				| \
													SYSTEM_WATCHDOG

#define SYSTEM_EVENTS_TO_CLEAR						SYSTEM_LINK_LOST					| \
													SYSTEM_CONNECTED_TO_SERVER          | \
													SYSTEM_CONNECTION_TO_SERVER_LOST	| \
													SYSTEM_CONFIGURED 					| \
													SYSTEM_TO_FW_UPDATE					| \
													SYSTEM_ADE9000_RECONFIG				| \
													SYSTEM_RAW_STATE_RUNNING			| \
													SYSTEM_RAW_STATE_STOPPED			| \
													SYSTEM_REG_STATE_RUNNING			| \
													SYSTEM_REG_STATE_STOPPED			| \
													SYSTEM_WATCHDOG

#define SYSTEM_INIT_TIME_SYNC_MS			30000   /* NTP has 15sec timeout, the system will wait double to give it time */
#define SYSTEM_AREA_DELETE_WAIT_MS			60000   /* tiem to wait until FW area is erased */

#define SYSTEM_WAIT_ETH_LINK_STATE_MSG		"Wait Ethernet link state\n"
#define SYSTEM_WAIT_SERVER_STATE_MSG		"Wait server state\n"
#define SYSTEM_NORMAL_STATE_MSG				"Normal state\n"
#define SYSTEM_FW_UPDATE_STATE_MSG			"FW update state\n"
#define SYSTEM_ERROR_STATE_MSG				"Error state\n"

#define SYSTEM_FW_UPDATE_STARTED_FLAG			0x9550D756
#define	SYSTEM_FW_UPDATE_DONE_FLAG 				0x3F2A9D57

/*******************************************************************************
 * Variables
 ******************************************************************************/

typedef struct wvf_names
{
	char* 				var_name;
	ade9000_phases_t 	var_phase;
	uint8_t				var_type_mask;
}wvf_names_t;

static system_states_t 	system_state;
static system_states_t 	system_state_before_disconnection;

static absl_timer_t	connection_timer;

static uint8_t 	enet_phy;

static bool	system_connected_eth;
static bool	system_connected_to_server;

static bool	energy_configured = false;

static absl_queue_t events_info_queue;
static absl_queue_t send_states_queue;
static absl_queue_t send_alets_queue;
static absl_queue_t fast_vars_stream_data_queue;
static absl_queue_t slow_vars_stream_data_queue;

/* WDOG queues */
static absl_queue_t cmd_watchdog_queue;
static absl_queue_t energy_watchdog_queue;
static absl_queue_t fw_update_watchdog_queue;
static absl_queue_t led_handler_watchdog_queue;
static absl_queue_t fast_stream_watchdog_queue;
static absl_queue_t slow_stream_watchdog_queue;
static absl_queue_t fast_vars_watchdog_queue;
static absl_queue_t slow_vars_watchdog_queue;
static absl_queue_t time_sync_watchdog_queue;
static absl_queue_t wdog_thread_watchdog_queue;
static absl_queue_t system_watchdog_queue;

static absl_event_t system_events;
static absl_event_t energy_events;
static absl_event_t fast_vars_read_events;
static absl_event_t slow_vars_read_events;
static absl_event_t fast_vars_stream_events;
static absl_event_t slow_vars_stream_events;
static absl_event_t cmd_events;
static absl_event_t fw_update_events;
static absl_event_t timesync_events;
static absl_event_t led_handler_events;
static absl_event_t watchdog_thread_events;

static absl_nvm_config_t*			nvm_config;
static absl_nvm_t					qspi_nvm;

static uint32_t	 state_events;

static uint32_t consistency_counter;

static uint32_t					error_amount;
static error_code_t*  			error_codes;
static state_data_to_send_t		send_data;

static event_info_t detecter_event_no_conn[MAX_EVENT_NO_CONN];
static uint32_t		no_conn_event_num;

static reg_address_t airms = {ADE9000_AIRMS, false, 0};
static reg_address_t avrms = {ADE9000_AVRMS, false, 0};
static reg_address_t birms = {ADE9000_BIRMS, false, 0};
static reg_address_t bvrms = {ADE9000_BVRMS, false, 0};
static reg_address_t cirms = {ADE9000_CIRMS, false, 0};
static reg_address_t cvrms = {ADE9000_CVRMS, false, 0};

static reg_address_t apf = {ADE9000_APF, false, 0};
static reg_address_t bpf = {ADE9000_BPF, false, 0};
static reg_address_t cpf = {ADE9000_CPF, false, 0};

static reg_address_t avthd = {ADE9000_AVTHD, false, 0};
static reg_address_t aithd = {ADE9000_AITHD, false, 0};
static reg_address_t bvthd = {ADE9000_BVTHD, false, 0};
static reg_address_t bithd = {ADE9000_BITHD, false, 0};
static reg_address_t cvthd = {ADE9000_CVTHD, false, 0};
static reg_address_t cithd = {ADE9000_CITHD, false, 0};

static reg_address_t awatthr_lo = {ADE9000_AWATTHR_LO, false, 0};
static reg_address_t awatthr_hi = {ADE9000_AWATTHR_HI, false, 0};
static reg_address_t bwatthr_lo = {ADE9000_BWATTHR_LO, false, 0};
static reg_address_t bwatthr_hi = {ADE9000_BWATTHR_HI, false, 0};
static reg_address_t cwatthr_lo = {ADE9000_CWATTHR_LO, false, 0};
static reg_address_t cwatthr_hi = {ADE9000_CWATTHR_HI, false, 0};

static reg_address_t avarhr_lo = {ADE9000_AVARHR_LO, false, 0};
static reg_address_t avarhr_hi = {ADE9000_AVARHR_HI, false, 0};
static reg_address_t bvarhr_lo = {ADE9000_BVARHR_LO, false, 0};
static reg_address_t bvarhr_hi = {ADE9000_BVARHR_HI, false, 0};
static reg_address_t cvarhr_lo = {ADE9000_CVARHR_LO, false, 0};
static reg_address_t cvarhr_hi = {ADE9000_CVARHR_HI, false, 0};

static reg_address_t avahr_lo = {ADE9000_AVAHR_LO, false, 0};
static reg_address_t avahr_hi = {ADE9000_AVAHR_HI, false, 0};
static reg_address_t bvahr_lo = {ADE9000_BVAHR_LO, false, 0};
static reg_address_t bvahr_hi = {ADE9000_BVAHR_HI, false, 0};
static reg_address_t cvahr_lo = {ADE9000_CVAHR_LO, false, 0};
static reg_address_t cvahr_hi = {ADE9000_CVAHR_HI, false, 0};

static reg_address_t afwatthr_lo = {ADE9000_AFWATTHR_LO, false, 0};
static reg_address_t afwatthr_hi = {ADE9000_AFWATTHR_HI, false, 0};
static reg_address_t bfwatthr_lo = {ADE9000_BFWATTHR_LO, false, 0};
static reg_address_t bfwatthr_hi = {ADE9000_BFWATTHR_HI, false, 0};
static reg_address_t cfwatthr_lo = {ADE9000_CFWATTHR_LO, false, 0};
static reg_address_t cfwatthr_hi = {ADE9000_CFWATTHR_HI, false, 0};

static reg_address_t afvarhr_lo = {ADE9000_AFVARHR_LO, false, 0};
static reg_address_t afvarhr_hi = {ADE9000_AFVARHR_HI, false, 0};
static reg_address_t bfvarhr_lo = {ADE9000_BFVARHR_LO, false, 0};
static reg_address_t bfvarhr_hi = {ADE9000_BFVARHR_HI, false, 0};
static reg_address_t cfvarhr_lo = {ADE9000_CFVARHR_LO, false, 0};
static reg_address_t cfvarhr_hi = {ADE9000_CFVARHR_HI, false, 0};

static reg_address_t afvahr_lo = {ADE9000_AFVAHR_LO, false, 0};
static reg_address_t afvahr_hi = {ADE9000_AFVAHR_HI, false, 0};
static reg_address_t bfvahr_lo = {ADE9000_BFVAHR_LO, false, 0};
static reg_address_t bfvahr_hi = {ADE9000_BFVAHR_HI, false, 0};
static reg_address_t cfvahr_lo = {ADE9000_CFVAHR_LO, false, 0};
static reg_address_t cfvahr_hi = {ADE9000_CFVAHR_HI, false, 0};

static reg_address_t oia = {ADE9000_OIA, false, 0};
static reg_address_t oib = {ADE9000_OIB, false, 0};
static reg_address_t oic = {ADE9000_OIC, false, 0};

static reg_address_t phnoload = {ADE9000_PHNOLOAD, false, 0};

static reg_address_t ipeak = {ADE9000_IPEAK, false, 0};
static reg_address_t vpeak = {ADE9000_VPEAK, false, 0};

static reg_address_t dipa = {ADE9000_DIPA, false, 0};
static reg_address_t dipb = {ADE9000_DIPB, false, 0};
static reg_address_t dipc = {ADE9000_DIPC, false, 0};

static reg_address_t swella = {ADE9000_SWELLA, false, 0};
static reg_address_t swellb = {ADE9000_SWELLB, false, 0};
static reg_address_t swellc = {ADE9000_SWELLC, false, 0};

static reg_address_t oistatus = {ADE9000_OISTATUS, false, 0};

static reg_address_t angl_va_vb = {ADE9000_ANGL_VA_VB, false, 0};
static reg_address_t angl_vb_vc = {ADE9000_ANGL_VB_VC, false, 0};
static reg_address_t angl_va_vc = {ADE9000_ANGL_VA_VC, false, 0};
static reg_address_t angl_ia_ib = {ADE9000_ANGL_IA_IB, false, 0};
static reg_address_t angl_ib_ic = {ADE9000_ANGL_IB_IC, false, 0};
static reg_address_t angl_ia_ic = {ADE9000_ANGL_IA_IC, false, 0};

static reg_address_t aperiod = {ADE9000_APERIOD, false, 0};
static reg_address_t bperiod = {ADE9000_BPERIOD, false, 0};
static reg_address_t cperiod = {ADE9000_CPERIOD, false, 0};

static reg_address_t isumrms = {ADE9000_ISUMRMS, false, 0};

static reg_address_t status1= {ADE9000_STATUS1, false, 0};

static ade9000_register_info_t ade9000_registers_info[ADE9000_VARS_MAXVALUE] =
{
	/* RMS */
	{SLOW_VAR_AIRMS_NAME, 	{INTENSITY, 1, {{0, 31, &airms}}}},
	{SLOW_VAR_AVRMS_NAME, 	{VOLTAGE,	1, {{0, 31, &avrms}}}},
	{SLOW_VAR_BIRMS_NAME,	{INTENSITY, 1, {{0, 31, &birms}}}},
	{SLOW_VAR_BVRMS_NAME, 	{VOLTAGE,	1, {{0, 31, &bvrms}}}},
	{SLOW_VAR_CIRMS_NAME, 	{INTENSITY, 1, {{0, 31, &cirms}}}},
	{SLOW_VAR_CVRMS_NAME, 	{VOLTAGE,	1, {{0, 31, &cvrms}}}},
	/* PF */
	{SLOW_VAR_APF_NAME, 	{POWER, 1, {{0, 31, &apf}}}},
	{SLOW_VAR_BPF_NAME, 	{POWER, 1, {{0, 31, &bpf}}}},
	{SLOW_VAR_CPF_NAME, 	{POWER, 1, {{0, 31, &cpf}}}},
	/* THD */
	{SLOW_VAR_AVTHD_NAME, 	{THD, 1, {{0, 31, &avthd}}}},
	{SLOW_VAR_AITHD_NAME, 	{THD, 1, {{0, 31, &aithd}}}},
	{SLOW_VAR_BVTHD_NAME, 	{THD, 1, {{0, 31, &bvthd}}}},
	{SLOW_VAR_BITHD_NAME, 	{THD, 1, {{0, 31, &bithd}}}},
	{SLOW_VAR_CVTHD_NAME, 	{THD, 1, {{0, 31, &cvthd}}}},
	{SLOW_VAR_CITHD_NAME, 	{THD, 1, {{0, 31, &cithd}}}},
	/* wattvar */
	{SLOW_VAR_AWATTHR_NAME,  	{WATVAR, 2, {{0, 31, &awatthr_lo}, 	{0, 31, &awatthr_hi}}}},
	{SLOW_VAR_BWATTHR_NAME,  	{WATVAR, 2, {{0, 31, &bwatthr_lo}, 	{0, 31, &bwatthr_hi}}}},
	{SLOW_VAR_CWATTHR_NAME, 	{WATVAR, 2, {{0, 31, &cwatthr_lo}, 	{0, 31, &cwatthr_hi}}}},
	{SLOW_VAR_AVARHR_NAME,		{WATVAR, 2, {{0, 31, &avarhr_lo}, 	{0, 31, &avarhr_hi}}}},
	{SLOW_VAR_BVARHR_NAME,		{WATVAR, 2, {{0, 31, &bvarhr_lo}, 	{0, 31, &bvarhr_hi}}}},
	{SLOW_VAR_CVARHR_NAME, 		{WATVAR, 2, {{0, 31, &cvarhr_lo}, 	{0, 31, &cvarhr_hi}}}},
	{SLOW_VAR_AVAHR_NAME,  		{WATVAR, 2, {{0, 31, &avahr_lo}, 	{0, 31, &avahr_hi}}}},
	{SLOW_VAR_BVAHR_NAME,  		{WATVAR, 2, {{0, 31, &bvahr_lo}, 	{0, 31, &bvahr_hi}}}},
	{SLOW_VAR_CVAHR_NAME, 		{WATVAR, 2, {{0, 31, &cvahr_lo}, 	{0, 31, &cvahr_hi}}}},
	{SLOW_VAR_AFWATTHR_NAME, 	{WATVAR, 2, {{0, 31, &afwatthr_lo}, {0, 31, &afwatthr_hi}}}},
	{SLOW_VAR_BFWATTHR_NAME, 	{WATVAR, 2, {{0, 31, &bfwatthr_lo}, {0, 31, &bfwatthr_hi}}}},
	{SLOW_VAR_CFWATTHR_NAME, 	{WATVAR, 2, {{0, 31, &cfwatthr_lo}, {0, 31, &cfwatthr_hi}}}},
	{SLOW_VAR_AFVARHR_NAME,  	{WATVAR, 2, {{0, 31, &afvarhr_lo}, 	{0, 31, &afvarhr_hi}}}},
	{SLOW_VAR_BFVARHR_NAME,  	{WATVAR, 2, {{0, 31, &bfvarhr_lo}, 	{0, 31, &bfvarhr_hi}}}},
	{SLOW_VAR_CFVARHR_NAME, 	{WATVAR, 2, {{0, 31, &cfvarhr_lo}, 	{0, 31, &cfvarhr_hi}}}},
	{SLOW_VAR_AFVAHR_NAME,  	{WATVAR, 2, {{0, 31, &afvahr_lo}, 	{0, 31, &afvahr_hi}}}},
	{SLOW_VAR_BFVAHR_NAME,  	{WATVAR, 2, {{0, 31, &bfvahr_lo}, 	{0, 31, &bfvahr_hi}}}},
	{SLOW_VAR_CFVAHR_NAME, 		{WATVAR, 2, {{0, 31, &cfvahr_lo}, 	{0, 31, &cfvahr_hi}}}},
	/*OI*/
	{SLOW_VAR_OIA_NAME, 		{OI, 1, {{0, 23, &oia}}}},
	{SLOW_VAR_OIB_NAME, 		{OI, 1, {{0, 23, &oib}}}},
	{SLOW_VAR_OIC_NAME, 		{OI, 1, {{0, 23, &oic}}}},
	/* no load */
	{SLOW_VAR_CFVANL_NAME, 		{VAR_NONE, 1, {{17, 17, &phnoload}}}},
	{SLOW_VAR_CFVARNL_NAME, 	{VAR_NONE, 1, {{16, 16, &phnoload}}}},
	{SLOW_VAR_CFWATTNL_NAME, 	{VAR_NONE, 1, {{15, 15, &phnoload}}}},
	{SLOW_VAR_CVANL_NAME, 		{VAR_NONE, 1, {{14, 14, &phnoload}}}},
	{SLOW_VAR_CVARNL_NAME, 		{VAR_NONE, 1, {{13, 13, &phnoload}}}},
	{SLOW_VAR_CWATTNL_NAME, 	{VAR_NONE, 1, {{12, 12, &phnoload}}}},
	{SLOW_VAR_BFVANL_NAME, 		{VAR_NONE, 1, {{11, 11, &phnoload}}}},
	{SLOW_VAR_BFVARNL_NAME, 	{VAR_NONE, 1, {{10, 10, &phnoload}}}},
	{SLOW_VAR_BFWATTNL_NAME, 	{VAR_NONE, 1, {{9,  9,  &phnoload}}}},
	{SLOW_VAR_BVANL_NAME, 		{VAR_NONE, 1, {{8,  8,  &phnoload}}}},
	{SLOW_VAR_BVARNL_NAME, 		{VAR_NONE, 1, {{7,  7,  &phnoload}}}},
	{SLOW_VAR_BWATTNL_NAME, 	{VAR_NONE, 1, {{6,  6,  &phnoload}}}},
	{SLOW_VAR_AFVANL_NAME, 		{VAR_NONE, 1, {{5,  5,  &phnoload}}}},
	{SLOW_VAR_AFVARNL_NAME, 	{VAR_NONE, 1, {{4,  4,  &phnoload}}}},
	{SLOW_VAR_AFWATTNL_NAME, 	{VAR_NONE, 1, {{3,  3,  &phnoload}}}},
	{SLOW_VAR_AVANL_NAME, 		{VAR_NONE, 1, {{2,  2,  &phnoload}}}},
	{SLOW_VAR_AVARNL_NAME, 		{VAR_NONE, 1, {{1,  1,  &phnoload}}}},
	{SLOW_VAR_AWATTNL_NAME, 	{VAR_NONE, 1, {{0,  0,  &phnoload}}}},

	{SLOW_VAR_IPPHASEA_NAME, 	{VAR_NONE,  1, {{24, 24, &ipeak}}}},
	{SLOW_VAR_IPPHASEB_NAME, 	{VAR_NONE,  1, {{25, 25, &ipeak}}}},
	{SLOW_VAR_IPPHASEC_NAME, 	{VAR_NONE,  1, {{26, 26, &ipeak}}}},
	{SLOW_VAR_IPEAK_NAME, 		{IPEAK, 	1, {{0,  23, &ipeak}}}},
	{SLOW_VAR_VPPHASEA_NAME, 	{VAR_NONE,  1, {{24, 24, &vpeak}}}},
	{SLOW_VAR_VPPHASEB_NAME, 	{VAR_NONE,  1, {{25, 25, &vpeak}}}},
	{SLOW_VAR_VPPHASEC_NAME, 	{VAR_NONE,  1, {{26, 26, &vpeak}}}},
	{SLOW_VAR_VPEAK_NAME, 		{VPEAK,   	1, {{0,  23, &vpeak}}}},

	{SLOW_VAR_DIPA_NAME, 		{DIP_SWELL,   1, {{0,  23, &dipa}}}},
	{SLOW_VAR_DIPB_NAME, 		{DIP_SWELL,   1, {{0,  23, &dipb}}}},
	{SLOW_VAR_DIPC_NAME, 		{DIP_SWELL,   1, {{0,  23, &dipc}}}},

	{SLOW_VAR_SWELLA_NAME, 		{DIP_SWELL,   1, {{0,  23, &swella}}}},
	{SLOW_VAR_SWELLB_NAME, 		{DIP_SWELL,   1, {{0,  23, &swellb}}}},
	{SLOW_VAR_SWELLC_NAME, 		{DIP_SWELL,   1, {{0,  23, &swellc}}}},

	{SLOW_VAR_OIPHASEA_NAME, 	{VAR_NONE,  1, {{0,  0, &oistatus}}}},
	{SLOW_VAR_OIPHASEB_NAME, 	{VAR_NONE,  1, {{1,  1, &oistatus}}}},
	{SLOW_VAR_OIPHASEC_NAME, 	{VAR_NONE,  1, {{2,  2, &oistatus}}}},

	{SLOW_VAR_ANGL_VA_VB_NAME, 	{ANGLE,  1, {{0,  15, &angl_va_vb}}}},
	{SLOW_VAR_ANGL_VB_VC_NAME, 	{ANGLE,  1, {{0,  15, &angl_vb_vc}}}},
	{SLOW_VAR_ANGL_VA_VC_NAME, 	{ANGLE,  1, {{0,  15, &angl_va_vc}}}},
	{SLOW_VAR_ANGL_IA_IB_NAME, 	{ANGLE,  1, {{0,  15, &angl_ia_ib}}}},
	{SLOW_VAR_ANGL_IB_IC_NAME, 	{ANGLE,  1, {{0,  15, &angl_ib_ic}}}},
	{SLOW_VAR_ANGL_IA_IC_NAME, 	{ANGLE,  1, {{0,  15, &angl_ia_ic}}}},

	{SLOW_VAR_APERIOD_NAME, 	{PERIOD,  1, {{0,  31, &aperiod}}}},
	{SLOW_VAR_BPERIOD_NAME, 	{PERIOD,  1, {{0,  31, &bperiod}}}},
	{SLOW_VAR_CPERIOD_NAME, 	{PERIOD,  1, {{0,  31, &cperiod}}}},

	{SLOW_VAR_ISUMRMS_NAME, 	{INTENSITY,  1, {{0, 31, &isumrms}}}},

	{SLOW_VAR_PHASE_SEQERR,   	{SEQERR, 1, {{18, 18, &status1}}}}
};

static manufacturing_t	default_manufacturing =
{
		"0-000000", {0x00, 0x60, 0x37, 0x00, 0x00, 0x00}, SYSTEM_MODEL,
		{
			{"ADSN", "0", "aingura", "v1.0.0"},
			{"DIEN", "0", "aingura", "v1.0.0"}
		}
};

static wvf_names_t wvf_var_info[WFB_AMOUNT] =
{
	{WFB_VAR_IA_STRING, PHASE_A, I_MASK},
	{WFB_VAR_VA_STRING, PHASE_A, V_MASK},
	{WFB_VAR_IB_STRING, PHASE_B, I_MASK},
	{WFB_VAR_VB_STRING, PHASE_B, V_MASK},
	{WFB_VAR_IC_STRING, PHASE_C, I_MASK},
	{WFB_VAR_VC_STRING, PHASE_C, V_MASK},
	{WFB_VAR_SA_STRING, PHASE_A, S_MASK},
	{WFB_VAR_SB_STRING, PHASE_B, S_MASK},
	{WFB_VAR_SC_STRING, PHASE_C, S_MASK},
	{WFB_VAR_PA_STRING, PHASE_A, P_MASK},
	{WFB_VAR_PB_STRING, PHASE_B, P_MASK},
	{WFB_VAR_PC_STRING, PHASE_C, P_MASK},
	{WFB_VAR_QA_STRING, PHASE_A, Q_MASK},
	{WFB_VAR_QB_STRING, PHASE_B, Q_MASK},
	{WFB_VAR_QC_STRING, PHASE_C, Q_MASK}
};

static ABSL_ALIGNED(16) waveform_data_t 	fast_vars_data;

/***************************************************************************
 * ENERGY TASK configuration
****************************************************************************/

static energy_task_states_t sensor_to_energy_state[SYSTEM_STATE_MAXVALUE] =
{
	ENERGY_TASK_IDLE ,				/* SYSTEM_INIT_STATE */
	ENERGY_TASK_IDLE,				/* SYSTEM_MANUFACTURING_STATE */
	ENERGY_TASK_IDLE,				/* SYSTEM_WAIT_ETH_LINK_STATE */
	ENERGY_TASK_IDLE,				/* SYSTEM_WAIT_SERVER_STATE */
	ENERGY_TASK_STATE_NORMAL,		/* SYSTEM_NORMAL_STATE */
	ENERGY_TASK_STATE_ERROR,		/* SYSTEM_ERROR_STATE */
	ENERGY_TASK_IDLE				/* SYSTEM_FW_UPDATE_STATE */
};	

static energy_sensor_init_conf_t energy_sensor_config =
{
	ABSL_SPI_ENERGY,
	ABSL_IRQ0_ENET_EVENT,
	ABSL_GPIO_ADE9000_RESET,
	&fast_vars_data.waveform_samples,
#ifdef DEBUG_PIN
	ABSL_GPIO_DEBUG,
#endif
	(void*)ade9000_event_info
};

static energy_thread_config_t energy_config =
{
	&energy_sensor_config, 				/* energy_sensor_conf */
	&system_events,						/* system_events */
	SYSTEM_CONFIGURED,					/* configured_event */
	SYSTEM_ADE9000_RECONFIG,			/* reconfig_event */
	SYSTEM_RAW_STATE_RUNNING,			/* wf_running */
	SYSTEM_RAW_STATE_STOPPED,			/* wf_stopped */
	SYSTEM_REG_STATE_RUNNING,			/* reg_running */
	SYSTEM_REG_STATE_STOPPED,			/* reg_stopped */
	&energy_events,						/* energy_events */
	&fast_vars_read_events,				/* fast_vars_events */
	&slow_vars_read_events,				/* slow_vars_events */
	VARS_CONFIG,						/* vars_config */
	VARS_READ,							/* vars_read */
	&fast_vars_stream_events,			/* fast_vars_stream_events */
	&slow_vars_stream_events,			/* fast_vars_stream_events */
	STREAM_CONNECT,						/* stream_connect */
	STREAM_DISCONNECT,					/* stream_disconnect */
	SENSOR_ADE9000,						/* sensor_label */
	SERVICE_REGISTERS,					/* slow_vars_service_label */
	SERVICE_RAW,						/* wvf_service_label */
	18,									/* time_between_vars_read_events_ms */
	&energy_watchdog_queue,				/* watchdog_queue */
	sensor_to_energy_state,				/* sensor_to_task_state */
	(void*)e_event_info,				/* event_info_array */
	false								/* energy_initialized */
};

static  absl_thread_t energy_thread;

/***************************************************************************
 * VARS READ configuration fast variables instance
****************************************************************************/
static bool system_get_fast_vars_config(void);

static fast_vars_config_t	fast_vars_config;

static float vars_queue_max_time_ms = 14;

static vars_read_thread_config_t fast_vars_read_thread_config =
{
	&fast_vars_read_events,		 				/* vars_events */
	&fast_vars_stream_events,					/* stream_event */	
	STREAM_DATA,								/* data_to_send_flag */	
	&energy_events,								/* energy_event */
	ENERGY_FAST_CONFIGURED,						/* vars_configured_flag */
	&fast_vars_stream_data_queue,				/* read_data_send_queue */
	&vars_queue_max_time_ms,					/* queue_send_wait_ms */
	ade9000_read_waveform,						/* vars_read_cb */
	system_get_fast_vars_config, 				/* vars_config_cb */
	(void*)&fast_vars_data,						/* read_vars */
	(void*)&fast_vars_config,					/* vars_config */
	&fast_vars_watchdog_queue,					/* watchdog_queue */
	vr_fast_event_info,							/* event_info_array */
	false				 						/* vars_read_initialized */
};

static vars_read_thread_data_t fast_vars_read_thread_data;

static vars_read_thread_t fast_vars_read =
{
	&fast_vars_read_thread_data,
	&fast_vars_read_thread_config
};

static  absl_thread_t fast_vars_read_thread;

/***************************************************************************
 * VARS READ configuration slow variables instance
****************************************************************************/
static bool system_get_slow_vars_config(void);

static slow_vars_data_t 	slow_vars_data;
static slow_vars_config_t	slow_vars_config;

static vars_read_thread_config_t slow_vars_read_thread_config =
{
	&slow_vars_read_events,		 				/* vars_events */
	&slow_vars_stream_events,					/* stream_event */	
	STREAM_DATA,								/* data_to_send_flag */
	&energy_events,								/* energy_event */
	ENERGY_SLOW_CONFIGURED,						/* vars_configured_flag */
	&slow_vars_stream_data_queue,				/* read_data_send_queue */
	&slow_vars_config.vars_read_period,			/* queue_send_wait_ms */
	ade9000_read_slow_variables,				/* vars_read_cb */
	system_get_slow_vars_config, 				/* vars_config_cb */
	(void*)&slow_vars_data,						/* read_vars */
	(void*)&slow_vars_config,					/* vars_config */
	&slow_vars_watchdog_queue,					/* watchdog_queue */
	vr_slow_event_info,							/* event_info_array */
	false				 						/* vars_read_initialized */
};

static vars_read_thread_data_t slow_vars_read_thread_data;

static vars_read_thread_t slow_vars_read =
{
	&slow_vars_read_thread_data,
	&slow_vars_read_thread_config
};

static  absl_thread_t slow_vars_read_thread;

/***************************************************************************
 * STREAMING TASK configuration fast variables instance
****************************************************************************/
static stream_data_config_t system_get_fast_vars_stream_config(void);

static waveform_data_t 		fast_vars_stream_data;

static  streaming_thread_config_t fast_streaming_config =
{
	ABSL_SOCKET_STREAM_FAST, 				/* stream_socket_index */
	&fast_vars_stream_events,			/* stream_events */
	&fast_vars_stream_data_queue,   	/* stream_data_queue */
	&energy_events,						/* service_event_group */
	ENERGY_FAST_CONNECTED,				/* connected_event */
	ENERGY_FAST_NOT_CONNECTED,			/* could_not_connect_event */
	sizeof(waveform_data_t),   			/* queue_data_size */
	5,									/* queue_data_amount */
	system_get_fast_vars_stream_config, /* stream_config_cb */
	(void*)&fast_vars_stream_data,		/* stream_data */
#ifdef DEBUG_PIN
	ABSL_GPIO_DEBUG,
#endif
	&fast_stream_watchdog_queue,		/* watchdog_queue */
	st_fast_event_info,					/* event_info_array */
	false								/* stream_initialized */
};

static streaming_thread_data_t fast_stream_thread_data;

static streaming_thread_t fast_stream =
{
	&fast_stream_thread_data,
	&fast_streaming_config
};

static  absl_thread_t fast_stream_thread;

/***************************************************************************
 * STREAMING TASK configuration slow variables instance
****************************************************************************/
static stream_data_config_t system_get_slow_vars_stream_config(void);

static slow_vars_data_t slow_vars_stream_data;

static  streaming_thread_config_t slow_streaming_config =
{
	ABSL_SOCKET_STREAM_SLOW, 				/* stream_socket_index */
	&slow_vars_stream_events,			/* stream_events */
	&slow_vars_stream_data_queue,   	/* stream_data_queue */
	&energy_events,						/* service_event_group */
	ENERGY_SLOW_CONNECTED,				/* connected_event */
	ENERGY_SLOW_NOT_CONNECTED,			/* could_not_connect_event */
	sizeof(slow_vars_data_t),   		/* queue_data_size */
	10,									/* queue_data_amount */
	system_get_slow_vars_stream_config, /* stream_config_cb */
	(void*)&slow_vars_stream_data,		/* stream_data */
#ifdef DEBUG_PIN
	ABSL_GPIO_DEBUG,
#endif
	&slow_stream_watchdog_queue,		/* watchdog_queue */
	st_slow_event_info,					/* event_info_array */
	false	
};

static streaming_thread_data_t slow_stream_thread_data;

static streaming_thread_t slow_stream =
{
	&slow_stream_thread_data,
	&slow_streaming_config
};

static  absl_thread_t slow_stream_thread;

/***************************************************************************
 * CMD TASK configuration
****************************************************************************/

static raw_config_t 		 raw_config;
static transmission_config_t raw_tx_config;

static service_config_t services_raw =
{
	ADE9000_SERVICE_RAW,
	json_get_raw_service_config,
	(void*)&raw_config,
	json_get_raw_transmission_config,
	&raw_tx_config
};

static vars_config_t 			vars_config;
static transmission_config_t 	vars_tx_config;

static service_config_t services_vars =
{
	ADE9000_SERVICE_REGISTERS,
	json_get_vars_service_config,
	(void*)&vars_config,
	json_get_vars_transmission_config,
	&vars_tx_config
};

static service_config_t* services_ade9000[SERVICE_MAXVALUE] = {&services_raw, &services_vars};

static energy_sensor_received_config_t ade9000_sensor_config;

static sensor_config_t energy_sensors[SENSOR_MAXVALUE] =
{
	{SENSOR_NAME,
	json_get_energy_sensor_config,
	(void*)&ade9000_sensor_config,
	SERVICE_MAXVALUE,
	services_ade9000}
};

static protocol_config_t protocol_config =
{
	ABSL_MQTT_COMMANDS,
	SENSOR_MAXVALUE,
	energy_sensors,
	FW_VERSION,
	NULL,
	NULL,
	(void*)mp_event_info,
	(void*)json_event_info
};

static cmd_task_states_t sensor_to_cmd_task_state[SYSTEM_STATE_MAXVALUE] =
{
	CMD_TASK_STATE_IDLE, 			/* SYSTEM_INIT_STATE */
	CMD_TASK_STATE_IDLE,			/* SYSTEM_MANUFACTURING_STATE */
	CMD_TASK_STATE_IDLE,			/* SYSTEM_WAIT_ETH_LINK_STATE */
	CMD_TASK_STATE_CONNECT,			/* SYSTEM_WAIT_SERVER_STATE */
	CMD_TASK_STATE_NORMAL,			/* SYSTEM_NORMAL_STATE */
	CMD_TASK_STATE_ERROR,			/* SYSTEM_ERROR_STATE */
	CMD_TASK_STATE_FW_UPDATE		/* SYSTEM_FW_UPDATE_STATE */
};	

static absl_event_t* sensors_events[SENSOR_MAXVALUE] = {&energy_events};

static uint32_t sensors_config_events[SENSOR_MAXVALUE] 	      = {ENERGY_CONFIGURATION};
static uint32_t sensors_config_reset_events[SENSOR_MAXVALUE]  = {ENERGY_CONFIG_RESET};

static uint32_t services_start_events[SERVICE_MAXVALUE] = {ENERGY_START_FAST_VARS, ENERGY_START_SLOW_VARS};
static uint32_t services_stop_events[SERVICE_MAXVALUE]  = {ENERGY_STOP_FAST_VARS, ENERGY_STOP_SLOW_VARS};

static uint32_t* sensors_service_start_events[SENSOR_MAXVALUE] = {services_start_events};
static uint32_t* sensors_service_stop_events[SENSOR_MAXVALUE]  = {services_stop_events};

static cmd_thread_config_t cmd_config =
{
	&protocol_config,									    /* cmd_protocol_config */
	NULL,													/* hw_version */
	&send_states_queue,										/* states_queue */
	&send_alets_queue,										/* alets_queue */
	&cmd_events,											/* cmd_events */
	&system_events,											/* system_events */
	SYSTEM_CONNECTED_TO_SERVER,								/* connected_event */
	SYSTEM_NO_SERVER,										/* no_server_event */
	SYSTEM_TO_FW_UPDATE,									/* fw_update_event */
	SYSTEM_CONNECTION_TO_SERVER_LOST,						/* disconnected_event */
	sensors_events,											/* sensor_event_groups */
	sensors_config_events,									/* config_events */
	sensors_config_reset_events,							/* reset_events */
	sensors_service_start_events,							/* start_events */
	sensors_service_stop_events,							/* stop_events */
	&timesync_events,										/* timesync_events */
	TIMESYNC_SYNC_MSG,										/* sync_event */
	&fw_update_events,										/* fw_update_events */
	FW_UPDATE_CHUNK_RECEIVED,								/* fw_file_chunk_received */
	FW_UPDATE_RECEPTION_FINISHED,							/* fw_file_reception_finished */
	&cmd_watchdog_queue,									/* watchdog_queue */
	sensor_to_cmd_task_state,								/* sensor_to_task_state */
	(void*)cmd_event_info,									/* event_info_array */
	false													/* cmd_initialized */
};

static  absl_thread_t cmd_thread;

/***************************************************************************
 * FW UPDATE TASK configuration
****************************************************************************/

static fw_update_task_states_t sensor_to_fw_update_task_state[SYSTEM_STATE_MAXVALUE] =
{
	FW_UPDATE_TASK_STATE_IDLE, 		/* SYSTEM_INIT_STATE */
	FW_UPDATE_TASK_STATE_IDLE,		/* SYSTEM_MANUFACTURING_STATE */
	FW_UPDATE_TASK_STATE_IDLE,		/* SYSTEM_WAIT_ETH_LINK_STATE */
	FW_UPDATE_TASK_STATE_IDLE,		/* SYSTEM_WAIT_SERVER_STATE */
	FW_UPDATE_TASK_STATE_IDLE,		/* SYSTEM_NORMAL_STATE */
	FW_UPDATE_TASK_STATE_IDLE,		/* SYSTEM_ERROR_STATE */
	FW_UPDATE_TASK_STATE_ACTIVE		/* SYSTEM_FW_UPDATE_STATE */
};	

static fw_update_thread_config_t fw_update_config =
{
	ABSL_NVM_CONFIG,
	{NVM_SECTION_PRIMARY, NVM_SECTION_SECONDARY, (void*)sfw_event_info},
	&fw_update_events,
	sensor_to_fw_update_task_state,
	4096,
	&system_events,
	SYSTEM_FW_AREA_ERASED,
	SYSTEM_FW_ERASE_NEEDED,
	SYSTEM_FW_UPDATE_DONE_FLAG,
	SYSTEM_FW_UPDATE_STARTED_FLAG,
	&fw_update_watchdog_queue,
	(void*)fwu_event_info,
	false
};

static absl_thread_t fw_update_thread;

/***************************************************************************
 * TIME SYNC TASK configuration
****************************************************************************/

static time_sync_thread_config_t timesync_config =
{
	ABSL_NTP_SYNC,
	&timesync_events,
	&system_events,
	SYSTEM_TIME_SYNC_FINISHED,
	&time_sync_watchdog_queue,
	(void*)ts_event_info,
	false
};

static absl_thread_t timesync_thread;

/***************************************************************************
 * LED TASK configuration
****************************************************************************/

#define INIT_STATE_LED_TOGGLE_AMOUNT 		2
#define MANUFACTURING_TOGGLE_AMOUNT 		8
#define ETHERNET_LINK_LED_TOGGLE_AMOUNT 	4
#define SERVER_WAIT_TOGGLE_AMOUNT 			6
#define NORMAL_STATE_TOGGLE_AMOUNT 			2
#define ERROR_STATE_TOGGLE_AMOUNT 			2
#define FW_UPDATE_TOGGLE_AMOUNT 			2

static toggle_state_t init_state_pattern[INIT_STATE_LED_TOGGLE_AMOUNT]   	 = {{LED_ON, 300},  {LED_OFF, 300}};
static toggle_state_t manufacturing_pattern[MANUFACTURING_TOGGLE_AMOUNT]     = {{LED_ON, 300},  {LED_OFF, 150}, 
																				{LED_ON, 300},  {LED_OFF, 150}, 
																				{LED_ON, 300},  {LED_OFF, 150}, 
																				{LED_ON, 300},  {LED_OFF, 2350}};
static toggle_state_t ethernet_link_pattern[ETHERNET_LINK_LED_TOGGLE_AMOUNT] = {{LED_ON, 300},  {LED_OFF, 150}, 
																				{LED_ON, 300},  {LED_OFF, 2250}};
static toggle_state_t server_wait_pattern[SERVER_WAIT_TOGGLE_AMOUNT]         = {{LED_ON, 300},  {LED_OFF, 150}, 
																				{LED_ON, 300},  {LED_OFF, 150}, 
																				{LED_ON, 300},  {LED_OFF, 1800}};
static toggle_state_t normal_state_pattern[NORMAL_STATE_TOGGLE_AMOUNT]       = {{LED_ON, 1000}, {LED_OFF, 1000}};
static toggle_state_t error_state_pattern[ERROR_STATE_TOGGLE_AMOUNT]         = {{LED_ON, 50},   {LED_OFF, 50}};
static toggle_state_t fw_update_pattern[FW_UPDATE_TOGGLE_AMOUNT]             = {{LED_ON, 300},  {LED_OFF, 700}};

static led_pattern_t state_pattern_table[SYSTEM_STATE_MAXVALUE] =
{
	{init_state_pattern, 	INIT_STATE_LED_TOGGLE_AMOUNT, 		0},		/* SYSTEM_INIT_STATE */
	{manufacturing_pattern, MANUFACTURING_TOGGLE_AMOUNT, 		0},		/* SYSTEM_MANUFACTURING_STATE */
	{ethernet_link_pattern, ETHERNET_LINK_LED_TOGGLE_AMOUNT, 	0},		/* SYSTEM_WAIT_ETH_LINK_STATE */
	{server_wait_pattern, 	SERVER_WAIT_TOGGLE_AMOUNT, 			0},		/* SYSTEM_WAIT_SERVER_STATE */
	{normal_state_pattern,	NORMAL_STATE_TOGGLE_AMOUNT, 		0},		/* SYSTEM_NORMAL_STATE */
	{error_state_pattern, 	ERROR_STATE_TOGGLE_AMOUNT, 			0},		/* SYSTEM_ERROR_STATE */
	{fw_update_pattern, 	FW_UPDATE_TOGGLE_AMOUNT, 			0}		/* SYSTEM_FW_UPDATE_STATE */
};	

static led_thread_config_t led_config =
{
	&led_handler_events,
	ABSL_GPIO_LED_USER,
	ABSL_GPIO_LED_USER_ENABLE,
	&system_state,
	state_pattern_table,
	&led_handler_watchdog_queue,
	false
};

static led_thread_data_t led_thread_data;

static led_thread_t status_led_thread =
{
	&led_thread_data,
	&led_config
};

static  absl_thread_t absl_status_led_thread;

/***************************************************************************
 * WDOG configuration
****************************************************************************/

typedef enum energy_threads
{
	CMD_THREAD = 0,
	ENERGY_THREAD,
	FW_UPDATE_THREAD,
	LED_HANDLER_THREAD,
	FAST_VARS_STREAMING_THREAD,
	SLOW_VARS_STREAMING_THREAD,
	FAST_VARS_READ_THREAD,
	SLOW_VARS_READ_THREAD,
	TIME_SYNC_THREAD,
	WATCHDOG_THREAD,
	SYSTEM_THREAD,
	ENERGY_THREADS_MAX_COUNT
}energy_threads_t;

#define CMD_BLOCKED_ERROR    				0xC87E2D93
#define ENERGY_BLOCKED_ERROR    			0xA1D04B7E
#define FW_UPDATE_BLOCKED_ERROR    			0x3C5F99A8
#define LED_HANDLER_BLOCKED_ERROR    		0xEAB1743C
#define FAST_VARS_STREAMING_BLOCKED_ERROR   0x9F6228D1
#define SLOW_VARS_STREAMING_BLOCKED_ERROR   0x134B7E66
#define FAST_VARS_READ_BLOCKED_ERROR    	0x48D9CE2A
#define SLOW_VARS_READ_BLOCKED_ERROR   		0xB06213FD
#define TIME_SYNC_BLOCKED_ERROR    			0xDC41A837
#define WATCHDOG_BLOCKED_ERROR    			0x7EF3C52B
#define SYSTEM_BLOCKED_ERROR    			0x20B89D4F

watchdog_thread_vars_t  watchdog_thread_vars[ENERGY_THREADS_MAX_COUNT] =
{
	{&cmd_events,		  	   CMD_WATCHDOG, 	  	 &cmd_watchdog_queue,         0, 0, CMD_BLOCKED_ERROR},
	{&energy_events, 	  	   ENERGY_WATCHDOG, 	 &energy_watchdog_queue,      0, 0, ENERGY_BLOCKED_ERROR},
	{&fw_update_events,   	   FW_UPDATE_WATCHDOG,   &fw_update_watchdog_queue,   0, 0, FW_UPDATE_BLOCKED_ERROR},
	{&led_handler_events, 	   LED_HANDLER_WATCHDOG, &led_handler_watchdog_queue, 0, 0, LED_HANDLER_BLOCKED_ERROR},
	{&fast_vars_stream_events, STREAM_WATCHDOG,      &fast_stream_watchdog_queue, 0, 0, FAST_VARS_STREAMING_BLOCKED_ERROR},
	{&slow_vars_stream_events, STREAM_WATCHDOG,      &slow_stream_watchdog_queue, 0, 0, SLOW_VARS_STREAMING_BLOCKED_ERROR},
	{&fast_vars_read_events,   VARS_WATCHDOG, 		 &fast_vars_watchdog_queue,   0, 0, FAST_VARS_READ_BLOCKED_ERROR},
	{&slow_vars_read_events,   VARS_WATCHDOG, 		 &slow_vars_watchdog_queue,   0, 0, SLOW_VARS_READ_BLOCKED_ERROR},
	{&timesync_events,         TIMESYNC_WATCHDOG, 	 &time_sync_watchdog_queue,   0, 0, TIME_SYNC_BLOCKED_ERROR},
	{&watchdog_thread_events,  WATCHDOG_THREAD_WDG,  &wdog_thread_watchdog_queue, 0, 0, WATCHDOG_BLOCKED_ERROR},
	{&system_events, 		   SYSTEM_WATCHDOG, 	 &system_watchdog_queue, 	  0, 0, SYSTEM_BLOCKED_ERROR}
};


static watchdog_config_t wdog_config =
{
	ABSL_WDOG1,
	&watchdog_thread_events,
	ENERGY_THREADS_MAX_COUNT,
	watchdog_thread_vars,
	&wdog_thread_watchdog_queue,
	false
};

static  absl_thread_t watchdog_thread;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void system_init_state(void);
static void system_wait_link_state(void);
static void system_wait_server_state(void);
static void system_normal_state(void);
static void system_error_state(void);
static void system_fw_update_state(void);

static void system_handler_events_handling(void);

static bool system_initialize(void);
static void system_handler_check_manufacturer_info(void);
static void system_link_check(void);
static void system_init_and_execute_led_thread(void);
static void system_init_threads(void);
static void system_init_thread_spawn(void);

static void 	watchdog_initialize(void);
static void 	system_error_reset_check(void);
static void 	system_handler_change_state(system_states_t _new_state, uint32_t _state_events, char* _dbg_message);
static void     system_handler_get_and_proccess_event(void);
static uint32_t system_handler_process_event(event_info_t _system_event, bool* _error_detected);
static void 	system_handler_send_no_conn_events(void);
static uint32_t system_handler_send_event(event_info_t _system_event, bool* _error_detected);
static void 	system_handler_evaluate_error(state_change_data_t _state_change);

static void system_handler_state_change_info_send(state_type_t _state_type, sensors_t _sensor, 
												  services_t _service, uint32_t _new_status);

static format_t system_get_stream_data_format(transmission_config_t* _tx_config);
static protocol_t system_get_stream_data_protocol(transmission_config_t* _tx_config);

/*******************************************************************************
 * Code
 ******************************************************************************/

void system_handler_reset(void* _arg)
{
	ABSL_UNUSED_ARG(_arg);

	absl_system_reboot();
}

/*!
 * @brief Function in charge of initializing all the
 * 		  threads of the system, before spawning them
 *
 */
void system_handler(void *arg)
{
	ABSL_UNUSED_ARG(arg);

    system_state = SYSTEM_INIT_STATE;
	system_connected_eth = false;
	system_connected_to_server = false;

	while(1)
	{
		switch(system_state)
		{
			case SYSTEM_INIT_STATE:
				system_init_state();
			break;

			case SYSTEM_WAIT_ETH_LINK_STATE:
				system_wait_link_state();
			break;

			case SYSTEM_WAIT_SERVER_STATE:
				system_wait_server_state();
			break;

			case SYSTEM_NORMAL_STATE:
				absl_timer_stop(&connection_timer);
				system_normal_state();
			break;
			
			case SYSTEM_ERROR_STATE:
				system_error_state();
			break;

			case SYSTEM_FW_UPDATE_STATE:
				system_fw_update_state();	
			break;

			default:
				absl_hardfault_handler(UNKNOWN_SWITCH_CASE_ERROR);
			break;
		} 
	} 
}

bool system_get_energy_config(void)
{
	return energy_configured;
}

uint32_t system_get_tc_primary_config(void)
{
	return ade9000_sensor_config.tc_primary_side;
}

uint32_t system_get_tc_secondary_config(void)
{
	return ade9000_sensor_config.tc_secondary_side;
}

energy_fundamental_freq_t system_get_fund_freq_config(void)
{
	energy_fundamental_freq_t fund_freq = FUNDAMENTAL_FREQ_INVALID;

	if(50 == ade9000_sensor_config.fundamental_frequency)
	{
		fund_freq = FUNDAMENTAL_FREQ_50_HZ;
	}
	else
	{
		fund_freq = FUNDAMENTAL_FREQ_60_HZ;
	}

	return fund_freq;
}

bool system_get_high_pass_filter_config(void)
{
	return ade9000_sensor_config.high_pass_filter_disable;
}

energy_hw_config_t system_get_hw_config(void)
{
	energy_hw_config_t hw_config = HW_CONF_UNKNOWN;

	if(!strcmp(HW_CONF_4WIRE_WYE_NEUTRAL_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_4_WIRE_WYE_NEUTRAL;
	}
	else if(!strcmp(HW_CONF_4WIRE_WYE_ISOLATED_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_4_WIRE_WYE_ISOLATED;
	}
	else if(!strcmp(HW_CONF_3WIRE_DELTA_PHASEB_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_3_WIRE_DELTA_PHASEB;
	}
	else if(!strcmp(HW_CONF_3WIRE_DELTA_ISOLATED_VB_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_3_WIRE_DELTA_ISOLATED_VB;
	}
	else if(!strcmp(HW_CONF_3WIRE_DELTA_ISOLATED_VA_VB_VC_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_3_WIRE_DELTA_ISOLATED_VA_VB_VC;
	}
	else if(!strcmp(HW_CONF_4WIRE_DELTA_NEUTRAL_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_4_WIRE_DELTA_NEUTRAL;
	}
	else if(!strcmp(HW_CONF_4WIRE_WYE_NOBLONDEL_NEUTRAL_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_4_WIRE_WYE_NONBLONDEL_NEUTRAL;
	}
	else if(!strcmp(HW_CONF_4WIRE_DELTA_NOBLONDEL_NEUTRAL_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_4_WIRE_DELTA_NONBLONDEL_NEUTRAL;
	}
	else if(!strcmp(HW_CONF_3WIRE_1PH_NEUTRAL_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_3_WIRE_1PH_NEUTRAL;
	}
	else if(!strcmp(HW_CONF_3WIRE_NETWORK_NEUTRAL_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_3_WIRE_NETWORK_NEUTRAL;
	}
	else if(!strcmp(HW_CONF_MULTIPLE_1PH_NEUTRAL_STRING, ade9000_sensor_config.hw_config))
	{
		hw_config = HW_CONF_MULTIPLE_1PH_NEUTRAL;
	}

	return hw_config;
}

uint32_t system_get_oi_threshold_config(void)
{
	return ade9000_sensor_config.OI_threshold;
}

uint32_t system_get_swell_threshold_config(void)
{
	return ade9000_sensor_config.SWELL_threshold;
}

uint32_t system_get_dip_threshold_config(void)
{
	return ade9000_sensor_config.DIP_threshold;
}

bool system_get_current_a_invert(void)
{
	return ade9000_sensor_config.ia_invert;
}

bool system_get_current_b_invert(void)
{
	return ade9000_sensor_config.ib_invert;
}

bool system_get_current_c_invert(void)
{
	return ade9000_sensor_config.ic_invert;
}

void system_get_igains(float* _igains)
{
	_igains[PHASE_A] = ade9000_sensor_config.ia_gain;
	_igains[PHASE_B] = ade9000_sensor_config.ib_gain;
	_igains[PHASE_C] = ade9000_sensor_config.ic_gain;
}

void system_get_vgains(float* _vgains)
{
	_vgains[PHASE_A] = ade9000_sensor_config.va_gain;
	_vgains[PHASE_B] = ade9000_sensor_config.vb_gain;
	_vgains[PHASE_C] = ade9000_sensor_config.vc_gain;
}

void system_get_pgains(float* _pgains)
{
	_pgains[PHASE_A] = 1;
	_pgains[PHASE_B] = 1;
	_pgains[PHASE_C] = 1;
}

void system_get_adc_redirect(uint32_t* _adc_redirect)
{
	_adc_redirect[VC_DIN] = ade9000_sensor_config.adc_redirect[VC_ADC_READ];
	_adc_redirect[VB_DIN] = ade9000_sensor_config.adc_redirect[VB_ADC_READ];
	_adc_redirect[VA_DIN] = ade9000_sensor_config.adc_redirect[VA_ADC_READ];
	_adc_redirect[IN_DIN] = ADC_DEFAULT_VALUE;
	_adc_redirect[IC_DIN] = ade9000_sensor_config.adc_redirect[IC_ADC_READ];
	_adc_redirect[IB_DIN] = ade9000_sensor_config.adc_redirect[IB_ADC_READ];
	_adc_redirect[IA_DIN] = ade9000_sensor_config.adc_redirect[IA_ADC_READ];
}

static bool system_get_fast_vars_config(void)
{
	fast_vars_config.variable_count = raw_config.raw_vars_amount;
	uint32_t var_pos = 0;

	fast_vars_config.pwr_p_q_calc[PHASE_A] = 0x00;
	fast_vars_config.pwr_p_q_calc[PHASE_B] = 0x00;
	fast_vars_config.pwr_p_q_calc[PHASE_C] = 0x00;

	fast_vars_config.calc_p_q = false;

	for(uint8_t var_index = 0; var_index < raw_config.raw_vars_amount; var_index++)
	{
		for(ade9000_wfb_samples_t var_info_index = 0; var_info_index < WFB_AMOUNT; var_info_index++)
		{
			if(!strcmp(wvf_var_info[var_info_index].var_name, raw_config.raw_vars_list[var_index]))
			{
				switch(wvf_var_info[var_info_index].var_type_mask)
				{
				case I_MASK:
					ade9000_set_i_buff_position(wvf_var_info[var_info_index].var_phase, var_pos);
					fast_vars_config.vars_to_calc[wvf_var_info[var_info_index].var_phase] |= wvf_var_info[var_info_index].var_type_mask;
					var_pos++;
					break;
				case V_MASK:
					ade9000_set_v_buff_position(wvf_var_info[var_info_index].var_phase, var_pos);
					fast_vars_config.vars_to_calc[wvf_var_info[var_info_index].var_phase] |= wvf_var_info[var_info_index].var_type_mask;
					var_pos++;
					break;
				case S_MASK:
					ade9000_set_s_buff_position(wvf_var_info[var_info_index].var_phase, var_pos);
					fast_vars_config.vars_to_calc[wvf_var_info[var_info_index].var_phase] |= wvf_var_info[var_info_index].var_type_mask;
					var_pos++;
					break;
				case P_MASK:
					ade9000_set_p_buff_position(wvf_var_info[var_info_index].var_phase, var_pos);
					fast_vars_config.vars_to_calc[wvf_var_info[var_info_index].var_phase] |= wvf_var_info[var_info_index].var_type_mask;
					var_pos++;
					break;
				case Q_MASK:
					ade9000_set_q_buff_position(wvf_var_info[var_info_index].var_phase, var_pos);
					fast_vars_config.vars_to_calc[wvf_var_info[var_info_index].var_phase] |= wvf_var_info[var_info_index].var_type_mask;
					var_pos++;
					break;
				default:
					/* Todo - error */
					break;
				}
			}
		}
	}

	for(ade9000_phases_t phase_index = 0; phase_index < PHASE_MAXNUM; phase_index++)
	{
		switch(fast_vars_config.vars_to_calc[phase_index])
		{
		case CALC_I:
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_NONE;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_i;
			break;
		case CALC_V:
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_NONE;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v;
			break;
		case CALC_V_I:
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_NONE;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_i;
			break;
		case CALC_S:
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_NONE;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_s;
			break;
		case CALC_S_P:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_P;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_s;
			break;
		case CALC_S_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_Q;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_s;
			break;
		case CALC_S_P_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_BOTH;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_s;
			break;
		case CALC_I_S:
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_NONE;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_i_s;
			break;
		case CALC_I_S_P:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_P;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_i_s;
			break;
		case CALC_I_S_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_Q;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_i_s;
			break;
		case CALC_I_S_P_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_BOTH;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_i_s;
			break;
		case CALC_V_S:
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_NONE;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_s;
			break;
		case CALC_V_S_P:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_P;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_s;
			break;
		case CALC_V_S_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_Q;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_s;
			break;
		case CALC_V_S_P_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_BOTH;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_s;
			break;
		case CALC_I_V_S:
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_NONE;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_i_s;
			break;
		case CALC_I_V_S_P:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_P;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_i_s;
			break;
		case CALC_I_V_S_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_Q;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_i_s;
			break;
		case CALC_I_V_S_P_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_BOTH;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_i_s;
			break;
		case CALC_P:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_P;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_powers;
			break;
		case CALC_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_Q;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_powers;
			break;
		case CALC_P_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_BOTH;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_powers;
			break;
		case CALC_I_P:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_P;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_i_powers;
			break;
		case CALC_I_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_Q;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_i_powers;
			break;
		case CALC_I_P_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_BOTH;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_i_powers;
			break;
		case CALC_I_V_P:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_P;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_i_powers;
			break;
		case CALC_I_V_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_Q;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_i_powers;
			break;
		case CALC_I_V_P_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_BOTH;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_i_powers;
			break;
		case CALC_V_P:
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_P;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_powers;
			break;
		case CALC_V_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_Q;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_powers;
			break;
		case CALC_V_P_Q:
			fast_vars_config.calc_p_q = true;
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_BOTH;
			fast_vars_config.var_calculation_cb[phase_index] = ade9000_calc_v_powers;
			break;
		case CALC_NONE:
			fast_vars_config.pwr_p_q_calc[phase_index] = CALC_PWR_NONE;
			fast_vars_config.var_calculation_cb[phase_index] = NULL;
			break;
		default:
			/* error */
			break;
		}
	}

	return true;
}

static stream_data_config_t system_get_fast_vars_stream_config(void)
{
	stream_data_config_t stream_config;

	stream_config.format = system_get_stream_data_format(&raw_tx_config);

	if(stream_config.format != FORMAT_UNKOWN)
	{
		stream_config.protocol = system_get_stream_data_protocol(&raw_tx_config);

		if(stream_config.protocol != PROTOCOL_UNKNOWN)
		{
			memcpy(stream_config.ip, raw_tx_config.settings.ip, 20);
			stream_config.port = raw_tx_config.settings.port;

			stream_config.data_size = 2 * sizeof(uint64_t); //timestamp start and end size
			stream_config.data_size += raw_config.raw_vars_amount * (sizeof(float) * WAFEFORM_DATA_SAMPLE_SIZE);
		}
		else
		{
			system_handler_process_event(sh_event_info[SH_EVENTS_UNKNOWN_FAST_COMM_PROTOCOL], NULL);
		}
	}
	else
	{
		system_handler_process_event(sh_event_info[SH_EVENTS_UNKNOWN_FAST_COMM_FORMAT], NULL);
	}

	return stream_config;
}

static bool system_get_slow_vars_config(void)
{
	bool all_regs_found = true;
	uint32_t reg_vars = 0;
	bool reg_found = false;

	uint32_t slow_vars_read_time;

	slow_vars_config.variable_count = vars_config.vars_amount;
	slow_vars_config.vars_read_period = vars_config.period;

	slow_vars_config.data_lost_time = vars_config.period * 2 * 1000000;

	/* Each variable takes around 200 microsecond to be read */
	slow_vars_read_time = slow_vars_config.variable_count * 200;

	/* The maximum time that slow vars reading should take, the configured period plus the time ti takes to read from the registers*/
	slow_vars_config.vars_read_time = (slow_vars_config.vars_read_period * 1000000) + slow_vars_read_time;
	/* Fast vars take around 15000 microsecond to be read, and they start again in around 4000 microsecond. Each time fast vars
	 * are been read slow vars are stopped, so if they are active this time need to be taken into account. */
	slow_vars_config.fast_vars_time = 15000 * ((slow_vars_read_time / 4000) + 1);

	if(slow_vars_config.data_lost_time < slow_vars_config.vars_read_time)
	{
		system_handler_process_event(sh_event_info[SH_ADE9000_REGISTER_HIGH_READ_TIME], NULL);
	}
	else if(slow_vars_config.data_lost_time < (slow_vars_config.vars_read_time + slow_vars_config.fast_vars_time))
	{
		system_handler_process_event(sh_event_info[SH_ADE9000_REGISTERS_AND_RAW_HIGH_READ_TIME], NULL);
	}

	for(uint32_t vars_index = 0; vars_index < vars_config.vars_amount; vars_index++)
	{
		for(ade9000_vars_t reg_vars_index = 0; reg_vars_index < ADE9000_VARS_MAXVALUE; reg_vars_index++)
		{
			if(!strcmp(vars_config.vars_list[vars_index], ade9000_registers_info[reg_vars_index].var_name))
			{
				slow_vars_config.registers[reg_vars] = ade9000_registers_info[reg_vars_index].register_var_entry;
				reg_vars++;
				reg_found = true;
			}
		}

		if(reg_found == false)
		{
			system_handler_process_event(sh_event_info[SH_ADE9000_REGISTER_NOT_FOUND], NULL);
			all_regs_found = false;
		}

		reg_found = false;
	}

	return all_regs_found;
}

float system_get_slow_vars_period(void)
{
	return slow_vars_config.vars_read_period;
}

static stream_data_config_t system_get_slow_vars_stream_config(void)
{
	stream_data_config_t stream_config;

	stream_config.format = system_get_stream_data_format(&vars_tx_config);

	if(stream_config.format != FORMAT_UNKOWN)
	{
		stream_config.protocol = system_get_stream_data_protocol(&vars_tx_config);

		if(stream_config.protocol != PROTOCOL_UNKNOWN)
		{
			memcpy(stream_config.ip, vars_tx_config.settings.ip, 20);
			stream_config.port = vars_tx_config.settings.port;

			stream_config.data_size = sizeof(uint64_t);
			for(uint32_t var_index = 0; var_index < vars_config.vars_amount; var_index++)
			{
				for(ade9000_vars_t reg_vars_index = 0; reg_vars_index < ADE9000_VARS_MAXVALUE; reg_vars_index++)
				{
					if(!strcmp(vars_config.vars_list[var_index], ade9000_registers_info[reg_vars_index].var_name))
					{
						stream_config.data_size += sizeof(uint32_t) * ade9000_registers_info[reg_vars_index].register_var_entry.reg_num;
					}
				}
			}
		}
		else
		{
			system_handler_process_event(sh_event_info[SH_EVENTS_UNKNOWN_SLOW_COMM_PROTOCOL], NULL);
		}
	}
	else
	{
		system_handler_process_event(sh_event_info[SH_EVENTS_UNKNOWN_SLOW_COMM_FORMAT], NULL);
	}

	return stream_config;
}

static format_t system_get_stream_data_format(transmission_config_t* _tx_config)
{
	format_t format = FORMAT_UNKOWN;

	if(!strcmp(TX_FORMAT_BIN_STRING, _tx_config->format))
	{
		format = FORMAT_BINARY;
	}
	else if(!strcmp(TX_FORMAT_JSON_STRING, _tx_config->format))
	{
		format = FORMAT_JSON;
	}
	else
	{
		/* Unknown */
	}

	return format;
}

static protocol_t system_get_stream_data_protocol(transmission_config_t* _tx_config)
{
	protocol_t protocol = PROTOCOL_UNKNOWN;

	if(!strcmp(TX_PROTOCOL_UDP_STRING, _tx_config->protocol))
	{
		protocol = PROTCOL_UDP;
	}
	else if(!strcmp(TX_PROTOCOL_TCP_STRING, _tx_config->protocol))
	{
		protocol = PROTCOL_TCP;
	}
	else
	{
		/* Unknown */
	}

	return protocol;
}

sync_type_t system_sync_get_sync_type(void)
{
	sync_type_t sync_type = SYNC_INVALID;

	char* sync_when =  mqtt_protocol_get_sync_when();

	if(!strcmp(SYNC_NOW_STRING, sync_when))
	{
		sync_type = SYNC_NOW;
	}
	else if(!strcmp(SYNC_POLL_STRING, sync_when))
	{
		sync_type = SYNC_POLL;
	}
	else
	{
		/* Invalid message */
	}

	return sync_type;
}

bool system_sync_get_sync_period(absl_time_t* _period)
{
	bool correct_period = false;
	char num_str[16] = {0};

	char* sync_interval =  mqtt_protocol_get_sync_interval();
	size_t len = 0;
	while (sync_interval[len] != '\0') len++;

	if (len >= 2 || sync_interval[len - 1] == 'h')
	{
		if (len - 1 < sizeof(num_str))
		{
			uint64_t u_interval;
			bool no_digit_found = false;
			for (size_t i = 0; i < len - 1; i++) {
				if (!isdigit((unsigned char)sync_interval[i]))
				{
					no_digit_found = true;
				}
				num_str[i] = sync_interval[i];
			}

			if(false == no_digit_found)
			{
				u_interval = (uint32_t) strtoul(num_str, NULL, 10);

				_period->seconds = u_interval * 3600;
				_period->nseconds = 0;

				correct_period = true;
			}
		}
	}

	return correct_period;
}

static void system_init_state(void)
{
	if(true == system_initialize())
	{
		system_state_before_disconnection = SYSTEM_WAIT_ETH_LINK_STATE;

		if(false == system_connected_eth)
		{
			system_handler_change_state(SYSTEM_WAIT_ETH_LINK_STATE, 0, SYSTEM_WAIT_ETH_LINK_STATE_MSG);
		}
		else
		{
			uint32_t  events;

			/* First time connected execute and wait first time sync */
			absl_event_set(&timesync_events, TIMESYNC_DO_SYNC);
			if(ABSL_EVENT_RV_OK != absl_event_timed_wait(&system_events, SYSTEM_TIME_SYNC_FINISHED, &events, SYSTEM_INIT_TIME_SYNC_MS))
			{
				system_handler_process_event(sh_event_info[SH_EVENTS_TIME_SYNC_PROCESS_ERROR], NULL);
			}

			system_handler_get_and_proccess_event();

			system_handler_change_state(SYSTEM_WAIT_SERVER_STATE, SYSTEM_WAIT_SERVER_STATE_EVENTS, SYSTEM_WAIT_SERVER_STATE_MSG);
		}
	}
	else
	{
		system_handler_change_state(SYSTEM_ERROR_STATE, SYSTEM_ERROR_STATE_EVENTS, "System initialization error\n");
	}
}

static void system_wait_link_state(void)
{
	system_link_check();

	if(system_connected_eth)
	{
		absl_timer_start(&connection_timer);
		system_handler_change_state(SYSTEM_WAIT_SERVER_STATE, SYSTEM_WAIT_SERVER_STATE_EVENTS, SYSTEM_WAIT_SERVER_STATE_MSG);
	}
}

static void system_wait_server_state(void)
{
	absl_event_set(&cmd_events, CMD_CONNECT);

	system_handler_events_handling();
}

static void system_normal_state(void)
{
	system_state_before_disconnection = SYSTEM_NORMAL_STATE;

	system_handler_events_handling();
}

static void system_error_state(void)
{
	system_state_before_disconnection = SYSTEM_ERROR_STATE;

	system_handler_events_handling();
}

static void system_fw_update_state(void)
{
	system_state_before_disconnection = SYSTEM_FW_UPDATE_STATE;

	system_handler_events_handling();
}

static void system_handler_events_handling(void)
{
	uint32_t 	events;

	absl_event_wait(&system_events, state_events, &events);


	if(SYSTEM_WATCHDOG == (events & SYSTEM_WATCHDOG))
	{
		consistency_counter++;
		absl_queue_send(&system_watchdog_queue, (void*)&consistency_counter, 1);
	}
	if(SYSTEM_CONNECTED_TO_SERVER == (events & SYSTEM_CONNECTED_TO_SERVER))
	{
		system_connected_to_server = true;
		events = 0;
		system_handler_state_change_info_send(DEVICE_STATE, 0, 0, (uint32_t)DEVICE_STATUS_ONLINE);
		uint32_t event_to_cmd = CMD_SEND_ONLINE;

		switch(system_state_before_disconnection)
		{
			case SYSTEM_WAIT_ETH_LINK_STATE:
			case SYSTEM_WAIT_SERVER_STATE:
			case SYSTEM_NORMAL_STATE:
				event_to_cmd |=  CMD_SEND_STATUS_CHANGE;
				system_handler_change_state(SYSTEM_NORMAL_STATE, SYSTEM_NORMAL_STATE_EVENTS, SYSTEM_NORMAL_STATE_MSG);
			break;
			case SYSTEM_FW_UPDATE_STATE:
				absl_system_get_fwu_clear_flag();
				system_handler_change_state(SYSTEM_NORMAL_STATE, SYSTEM_NORMAL_STATE_EVENTS, SYSTEM_NORMAL_STATE_MSG);
			break;
			case SYSTEM_ERROR_STATE:
				system_handler_change_state(SYSTEM_ERROR_STATE, SYSTEM_ERROR_STATE_EVENTS, SYSTEM_ERROR_STATE_MSG);
			break;
			default:
				system_handler_change_state(SYSTEM_ERROR_STATE, SYSTEM_ERROR_STATE_EVENTS, "INORRECT STATE TO CHANGE!!\n");
			break;
		}

		absl_event_set(&cmd_events, event_to_cmd);
		system_handler_send_no_conn_events();
	}
	if(SYSTEM_LINK_LOST == (events & SYSTEM_LINK_LOST))
	{
		events = 0;
		system_connected_eth = false;
		system_connected_to_server = false;

		system_handler_change_state(SYSTEM_WAIT_ETH_LINK_STATE, 0, SYSTEM_WAIT_ETH_LINK_STATE_MSG);

		if(SYSTEM_FW_UPDATE_STATE == system_state_before_disconnection)
		{
			absl_event_set(&cmd_events, CMD_SERVER_DISCONNECT);

			absl_system_get_fwu_clear_flag();
			system_handler_process_event(sh_event_info[SH_EVENTS_FW_UPDATE_DISCONNECTION], NULL);
			absl_event_set(&fw_update_events, FW_UPDATE_RESET_FW_UPDATE);

			if(ABSL_EVENT_RV_OK != absl_event_timed_wait(&system_events, SYSTEM_FW_AREA_ERASED, &events, SYSTEM_AREA_DELETE_WAIT_MS))
			{
				system_handler_process_event(sh_event_info[SH_EVENTS_FLASH_AREA_DELETE_ERROR], NULL);
			}
		}
	}
	if (SYSTEM_NO_SERVER == (events & SYSTEM_NO_SERVER))
	{
		/* No server found with the actual IP, reassign IP in case there is a new server */
		absl_phy_assign_ip(enet_phy);
		system_handler_change_state(SYSTEM_WAIT_SERVER_STATE, SYSTEM_WAIT_SERVER_STATE_EVENTS, SYSTEM_WAIT_SERVER_STATE_MSG);
	}
	if(SYSTEM_CONFIGURED == (events & SYSTEM_CONFIGURED))
	{
		energy_configured = true;
		system_handler_state_change_info_send(SENSOR_STATE, SENSOR_ADE9000, 0, (uint32_t)SENSOR_STATUS_CONFIGURED);
		absl_event_set(&cmd_events, CMD_SEND_STATUS_CHANGE);
	}
	if (SYSTEM_RAW_STATE_RUNNING == (events & SYSTEM_RAW_STATE_RUNNING))
	{
		system_handler_state_change_info_send(SERVICE_STATE, SENSOR_ADE9000, SERVICE_RAW, (uint32_t)SERVICE_STATUS_RUNNING);
		absl_event_set(&cmd_events, CMD_SEND_STATUS_CHANGE);
	}
	if (SYSTEM_RAW_STATE_STOPPED == (events & SYSTEM_RAW_STATE_STOPPED))
	{
		system_handler_state_change_info_send(SERVICE_STATE, SENSOR_ADE9000, SERVICE_RAW, (uint32_t)SERVICE_STATUS_IDLE);
		absl_event_set(&cmd_events, CMD_SEND_STATUS_CHANGE);
	}
	if (SYSTEM_REG_STATE_RUNNING == (events & SYSTEM_REG_STATE_RUNNING))
	{
		system_handler_state_change_info_send(SERVICE_STATE, SENSOR_ADE9000, SERVICE_REGISTERS, (uint32_t)SERVICE_STATUS_RUNNING);
		absl_event_set(&cmd_events, CMD_SEND_STATUS_CHANGE);
	}
	if (SYSTEM_REG_STATE_STOPPED == (events & SYSTEM_REG_STATE_STOPPED))
	{
		system_handler_state_change_info_send(SERVICE_STATE, SENSOR_ADE9000, SERVICE_REGISTERS, (uint32_t)SERVICE_STATUS_IDLE);
		absl_event_set(&cmd_events, CMD_SEND_STATUS_CHANGE);
	}
	if (SYSTEM_CONNECTION_TO_SERVER_LOST == (events & SYSTEM_CONNECTION_TO_SERVER_LOST))
	{
		system_connected_to_server = false;
		events = 0;

		absl_timer_start(&connection_timer);
		system_handler_change_state(SYSTEM_WAIT_SERVER_STATE, SYSTEM_WAIT_SERVER_STATE_EVENTS, SYSTEM_WAIT_SERVER_STATE_MSG);

		if(SYSTEM_FW_UPDATE_STATE == system_state_before_disconnection)
		{
			absl_system_get_fwu_clear_flag();
			system_handler_process_event(sh_event_info[SH_EVENTS_FW_UPDATE_DISCONNECTION], NULL);
			absl_event_set(&fw_update_events, FW_UPDATE_RESET_FW_UPDATE);
			if(ABSL_EVENT_RV_OK != absl_event_timed_wait(&system_events, SYSTEM_FW_AREA_ERASED, &events, SYSTEM_AREA_DELETE_WAIT_MS))
			{
				system_handler_process_event(sh_event_info[SH_EVENTS_FLASH_AREA_DELETE_ERROR], NULL);
			}
		}
	}
	if(SYSTEM_TO_FW_UPDATE == (events & SYSTEM_TO_FW_UPDATE))
	{
		absl_system_set_fwu_flag(SYSTEM_FW_UPDATE_STARTED_FLAG);

		system_handler_change_state(SYSTEM_FW_UPDATE_STATE, SYSTEM_FW_UPDATE_EVENTS, SYSTEM_FW_UPDATE_STATE_MSG);
	}
	if(SYSTEM_ADE9000_RECONFIG == (events & SYSTEM_ADE9000_RECONFIG))
	{
		energy_configured = false;
		if(true == system_connected_to_server)
		{
			system_handler_state_change_info_send(SENSOR_STATE, SENSOR_ADE9000, 0, (uint32_t)SENSOR_STATUS_IDLE);
			system_handler_state_change_info_send(SERVICE_STATE, SENSOR_ADE9000, SERVICE_RAW, (uint32_t)SERVICE_STATUS_IDLE);
			system_handler_state_change_info_send(SERVICE_STATE, SENSOR_ADE9000, SERVICE_REGISTERS, (uint32_t)SERVICE_STATUS_IDLE);
			absl_event_set(&cmd_events, CMD_SEND_STATUS_CHANGE);
		}
	}
	if(SYSTEM_EVENT_TO_PROCESS == (events & SYSTEM_EVENT_TO_PROCESS))
	{
		system_handler_get_and_proccess_event();
	}
	if(SYSTEM_FW_ERASE_NEEDED == (events & SYSTEM_FW_ERASE_NEEDED))
	{
		/* FW update could not done, because flash need to be erased.
		   While erasing connection could be lost, so a disconnection is forced
		   just to connecabsl_system_initted after the erase of the memory */
		absl_system_get_fwu_clear_flag();

		absl_event_set(&cmd_events, CMD_SERVER_DISCONNECT);
		system_connected_to_server = false;
		system_handler_change_state(SYSTEM_WAIT_SERVER_STATE, SYSTEM_WAIT_SERVER_STATE_EVENTS, SYSTEM_WAIT_SERVER_STATE_MSG);

		/* wait until flash ares is erased */
		absl_event_set(&fw_update_events, FW_UPDATE_RESET_FW_UPDATE);
		if(ABSL_EVENT_RV_OK != absl_event_timed_wait(&system_events, SYSTEM_FW_AREA_ERASED, &events, SYSTEM_AREA_DELETE_WAIT_MS))
		{
			system_handler_process_event(sh_event_info[SH_EVENTS_FLASH_AREA_DELETE_ERROR], NULL);
		}

		system_handler_process_event(sh_event_info[SH_EVENTS_FW_UPDATE_AREA_ERASED], NULL);
	}
}

static void connection_timeout_cb(void* arg)
{
	ABSL_UNUSED_ARG(arg);

	absl_system_reboot();
}


static bool system_initialize(void)
{
	bool init_done = false;

	absl_time_t	connection_timeout;

	enet_phy = PHY_AMOUNT - 1;
	no_conn_event_num = 0;

	absl_system_init();

	nvm_config = absl_config_get_nvm_conf(ABSL_NVM_CONFIG);

	if(ABSL_NVM_RV_OK == absl_nvm_init(&qspi_nvm, nvm_config))
	{
		if(ABSL_EVENT_RV_OK == absl_event_create(&system_events))
		{
			if((ABSL_QUEUE_RV_OK == absl_queue_create(&events_info_queue, sizeof(event_info_t), 5)) &&
			   (ABSL_QUEUE_RV_OK == absl_queue_create(&send_states_queue, sizeof(state_change_data_t), 5)) &&
			   (ABSL_QUEUE_RV_OK == absl_queue_create(&send_alets_queue, sizeof(alert_data_t), 5)) &&
			   (ABSL_QUEUE_RV_OK == absl_queue_create(&system_watchdog_queue, sizeof(consistency_counter), 1)))
			{
				event_handler_init(energy_sensors, &system_events, SYSTEM_EVENT_TO_PROCESS, &events_info_queue);
				watchdog_initialize();
				system_error_reset_check();

				system_init_and_execute_led_thread();

				system_handler_check_manufacturer_info();

				connection_timeout.seconds = 30;
				connection_timeout.nseconds = 0;
				absl_timer_create(&connection_timer, &connection_timeout_cb, NULL,
								connection_timeout, false, true);

				system_init_threads();
				system_init_thread_spawn();

				if(PHY_AMOUNT == absl_phy_get_init_phy_amount())
				{
					if(ABSL_PHY_RV_LINK_UP == absl_phy_check_link_state(enet_phy))
					{
						system_connected_eth = true;
					}
					absl_phy_set_link_down_event(enet_phy, &system_events, SYSTEM_LINK_LOST);

					consistency_counter = 0;

					init_done = true;
				}
			}
			else
			{
				absl_hardfault_handler(QUEUE_CREATE_ERROR);
			}
		}
		else
		{
			absl_hardfault_handler(EVENT_CREATE_ERROR);
		}
	}

	return init_done;
}

static void system_handler_check_manufacturer_info(void)
{
	manufacturing_t* manufacturing_data = 0;
	manufacturing_config_t manufacturing_config;

	manufacturing_config.nvm = &qspi_nvm;
	manufacturing_config.sector_index = NVM_SECTION_FACTORY_CONF;
	manufacturing_config.default_manufacturing = &default_manufacturing;
	manufacturing_config.system_model = (char*)SYSTEM_MODEL;
	manufacturing_config.manufacturing_events_info_array = (void*)manu_event_info;

	manufacturing_initialize(&manufacturing_config);

	if(true != manufacturing_get_data(manufacturing_data))
	{
		system_state = SYSTEM_MANUFACTURING_STATE;
		manufacturing(manufacturing_data);
		system_state = SYSTEM_INIT_STATE;
	}

	protocol_config.device_ID = manufacturing_data->id_mumber;
	absl_hw_config_assign_phy_mac(manufacturing_data->mac_address);
	protocol_config.model = manufacturing_data->model;
	cmd_config.hw_version = (void*)&manufacturing_data->hw_version[0];
}

static void system_link_check(void)
{
	if(ABSL_PHY_RV_LINK_UP == absl_phy_check_link(enet_phy))
	{
		absl_phy_assign_ip(enet_phy);
		system_connected_eth = true;
	}
	else
	{
		system_connected_eth = false;
	}
}

static void system_init_and_execute_led_thread(void)
{
	if(!led_handler_initialize(&status_led_thread))
	{
		absl_debug_printf("LED initialization failed!\r\n");
		absl_hardfault_handler(THREAD_INIT_ERROR);
	}

	if (ABSL_THREAD_RV_OK != absl_thread_create(&absl_status_led_thread, "LED task", led_handler_task,
						LED_HANDLER_PRIORITY, LED_HANDLER_STACK_SIZE, &status_led_thread))
	{
		absl_debug_printf("FW update task creation failed!.\r\n");
		absl_hardfault_handler(THREAD_CREATE_ERROR);
	}
}

/*!
 * @brief Funtion in charge of spawning all the
 * 		  threads of the system
 *
 */
static void system_init_threads(void)
{
	if(!energy_task_initialize(&energy_config))
	{
		absl_debug_printf("Energy task initialization failed!\r\n");
		absl_hardfault_handler(THREAD_INIT_ERROR);
	}

	if(!vars_read_task_initialize(&fast_vars_read_thread_data, &fast_vars_read_thread_config))
	{
		absl_debug_printf("Fast vars reading task initialization failed!\r\n");
		absl_hardfault_handler(THREAD_INIT_ERROR);
	}
	
	if(!vars_read_task_initialize(&slow_vars_read_thread_data, &slow_vars_read_thread_config))
	{
		absl_debug_printf("Slow vars reading task initialization failed!\r\n");
		absl_hardfault_handler(THREAD_INIT_ERROR);
	}

	if(!streaming_task_initialize(&fast_stream_thread_data, &fast_streaming_config))
	{
		absl_debug_printf("Fast vars streaming task initialization failed!\r\n");
		absl_hardfault_handler(THREAD_INIT_ERROR);
	}

	if(!streaming_task_initialize(&slow_stream_thread_data, &slow_streaming_config))
	{
		absl_debug_printf("Slow vars streaming task initialization failed!\r\n");
		absl_hardfault_handler(THREAD_INIT_ERROR);
	}

	if(!cmd_task_initialize(&cmd_config))
	{
		absl_debug_printf("Commands task initialization failed!\r\n");
		absl_hardfault_handler(THREAD_INIT_ERROR);
	}

	if(!fw_update_task_initialize(&fw_update_config))
	{
		absl_debug_printf("FW update task initialization failed!\r\n");
		absl_hardfault_handler(THREAD_INIT_ERROR);
	}

	if(!time_sync_initialize(&timesync_config))
	{
		absl_debug_printf("Time sync initialization failed!\r\n");
		absl_hardfault_handler(THREAD_INIT_ERROR);
	}

	system_handler_get_and_proccess_event();
}

static void system_init_thread_spawn(void)
{
	absl_debug_printf("Spawning threads!\n");

	if (ABSL_THREAD_RV_OK != absl_thread_create(&timesync_thread, "timesync", time_sync_task,
											TIMESYNC_PRIORITY, TIMESYNC_STACK_SIZE, &system_state))
	{
		absl_debug_printf("Timesync creation failed!.\r\n");
		absl_hardfault_handler(THREAD_CREATE_ERROR);
	}

	if (ABSL_THREAD_RV_OK != absl_thread_create(&energy_thread, "Energy task", energy_task,
						   ENERGY_TASK_PRIORITY, ENERGY_TASK_STACK_SIZE, &system_state))
	{
		absl_debug_printf("Energy task creation failed!.\r\n");
		absl_hardfault_handler(THREAD_CREATE_ERROR);
	}

	if (ABSL_THREAD_RV_OK != absl_thread_create(&fast_vars_read_thread, "Fast vars read", vars_read_task,
											FAST_READ_TASK_PRIORITY, FAST_READ_TASK_STACK_SIZE, (void*)&fast_vars_read))
	{
		absl_debug_printf("Fast vars reading task creation failed!.\r\n");
		absl_hardfault_handler(THREAD_CREATE_ERROR);
	}

	if (ABSL_THREAD_RV_OK != absl_thread_create(&slow_vars_read_thread, "Slow vars read", vars_read_task,
						   				    SLOW_READ_TASK_PRIORITY, SLOW_READ_TASK_STACK_SIZE, (void*)&slow_vars_read))
	{
		absl_debug_printf("Slow vars task creation failed!.\r\n");
		absl_hardfault_handler(THREAD_CREATE_ERROR);
	}

	if (ABSL_THREAD_RV_OK != absl_thread_create(&fast_stream_thread, "fast stream task", streaming_task,
						   FAST_STREAM_TASK_PRIORITY, FAST_STREAM_TASK_STACK_SIZE, (void*)&fast_stream))
	{
		absl_debug_printf("Streaming task creation failed!.\r\n");
		absl_hardfault_handler(THREAD_CREATE_ERROR);
	}

	if (ABSL_THREAD_RV_OK != absl_thread_create(&slow_stream_thread, "slow stream task", streaming_task,
						   SLOW_STREAM_TASK_PRIORITY, SLOW_STREAM_TASK_STACK_SIZE, (void*)&slow_stream))
	{
		absl_debug_printf("Streaming task creation failed!.\r\n");
		absl_hardfault_handler(THREAD_CREATE_ERROR);
	}

	if (ABSL_THREAD_RV_OK != absl_thread_create(&cmd_thread, "Commands task", cmd_task,
							   CMD_TASK_PRIORITY, CMD_TASK_STACK_SIZE, &system_state))
	{
		absl_debug_printf("Commands task creation failed!.\r\n");
		absl_hardfault_handler(THREAD_CREATE_ERROR);
	}

	if (ABSL_THREAD_RV_OK != absl_thread_create(&fw_update_thread, "FW update task", fw_update_task,
						   FW_UPDATE_TASK_PRIORITY, FW_UPDATE_TASK_STACK_SIZE, &system_state))
	{
		absl_debug_printf("FW update task creation failed!.\r\n");
		absl_hardfault_handler(THREAD_CREATE_ERROR);
	}

	if (ABSL_THREAD_RV_OK != absl_thread_create(&watchdog_thread, "Watchdog task", watchdog_task,
											WATCHDOG_PRIORITY, WATCHDOG_STACK_SIZE, NULL))
	{
		absl_debug_printf("FW update task creation failed!.\r\n");
		absl_hardfault_handler(THREAD_CREATE_ERROR);
	}
}

/*!
 * @brief Function in charge of initializing all the
 * 		  non thread modules, and executes or needed
 * 		  actions before spawning the threads
 *
 */
static void watchdog_initialize(void)
{
	absl_system_reset_t reset_reason;

	if(false == watchdog_init(&wdog_config))
	{
		absl_hardfault_handler(WDOG_INIT_ERROR);
	}

	reset_reason = absl_system_get_reset_reason();

	switch (reset_reason)
	{
	case ABSL_SYSTEM_POWERUP_RESET:
		system_handler_process_event(sh_event_info[SH_EVENTS_POWERUP_RESET], NULL);
		break;
	case ABSL_SYSTEM_SOFTWARE_RESET:
		system_handler_process_event(sh_event_info[SH_EVENTS_SOFTWARE_RESET], NULL);
		break;
	case ABSL_SYSTEM_WDOG_TIMEOUT_RESET:
		absl_debug_printf("\nWARNING!!! Watchdog timeout reset!!!\n");
		system_handler_process_event(sh_event_info[SH_EVENTS_WDOG_RESET], NULL);
		break;
	case ABSL_SYSTEM_SECURITY_RESET:
		absl_debug_printf("\nWARNING!!! Security reset!!!\n");
		system_handler_process_event(sh_event_info[SH_EVENTS_SECURITY_RESET], NULL);
		break;
	case ABSL_SYSTEM_JTAG_HW_RESET:
		system_handler_process_event(sh_event_info[SH_EVENTS_JTAG_HW_RESET], NULL);
		break;
	case ABSL_SYSTEM_JTAG_SW_RESET:
		system_handler_process_event(sh_event_info[SH_EVENTS_JTAG_SW_RESET], NULL);
		break;
	case ABSL_SYSTEM_TEMPSENSE_RESET:
		absl_debug_printf("\nWARNING!!! Temperature sensor reset!!!\n");
		system_handler_process_event(sh_event_info[SH_EVENTS_TEMPSENSE_RESET], NULL);
		break;
	default:
		break;
	}

	watchdog_run();
}

static void system_error_reset_check(void)
{
	uint32_t hardfault_error = absl_system_get_hf_error_flag();

	switch(hardfault_error)
	{
	case HARDFAULT_ERROR:
		system_handler_process_event(sh_event_info[SH_GENERIC_HF_ERROR_RESET], NULL);
		break;
	case NMI_HARDFAULT_ERROR:
		system_handler_process_event(sh_event_info[SH_NMI_HF_ERROR_RESET], NULL);
		break;
	case MEM_MAN_HARDFAULT_ERROR:
		system_handler_process_event(sh_event_info[SH_MEM_MAN_HF_ERROR_RESET], NULL);
		break;
	case BUS_HARDFAULT_ERROR:
		system_handler_process_event(sh_event_info[SH_BUS_HF_ERROR_RESET], NULL);
		break;
	case USAGE_HARDFAULT_ERROR:
		system_handler_process_event(sh_event_info[SH_USAGE_HF_ERROR_RESET], NULL);
		break;
	case SVC_HARDFAULT_ERROR:
		system_handler_process_event(sh_event_info[SH_SVC_HF_ERROR_RESET], NULL);
		break;
	case DEBUGMON_HARDFAULT_ERROR:
		system_handler_process_event(sh_event_info[SH_DEBUGMON_HF_ERROR_RESET], NULL);
		break;
	case PEND_SV_HARDFAULT_ERROR:
		system_handler_process_event(sh_event_info[SH_PENDSV_HF_ERROR_RESET], NULL);
		break;
	case SYSTICK_HARDFAULT_ERROR:
		system_handler_process_event(sh_event_info[SH_SYSTICK_HF_ERROR_RESET], NULL);
		break;
	case INT_HANDLER_HARDFAULT_ERROR:
		system_handler_process_event(sh_event_info[SH_INTHAND_HF_ERROR_RESET], NULL);
		break;
	case STACK_OVERFLOW_FLAG:
		system_handler_process_event(sh_event_info[SH_STACKOF_HF_ERROR_RESET], NULL);
		break;
	case MALLOC_FAILED_FLAG:
		system_handler_process_event(sh_event_info[SH_MALLOC_HF_ERROR_RESET], NULL);
		break;
	case ASSERTION_ERROR:
		system_handler_process_event(sh_event_info[SH_ASSERT_HF_ERROR_RESET], NULL);
		break;
	case THREAD_CREATE_ERROR:
		system_handler_process_event(sh_event_info[SH_THREAD_CREATE_HF_ERROR_RESET], NULL);
		break;
	case THREAD_INIT_ERROR:
		system_handler_process_event(sh_event_info[SH_THREAD_INIT_HF_ERROR_RESET], NULL);
		break;
	case THREAD_NOT_INIT_ERROR:
		system_handler_process_event(sh_event_info[SH_THREAD_NOT_INIT_HF_ERROR_RESET], NULL);
		break;
	case WDOG_INIT_ERROR:
		system_handler_process_event(sh_event_info[SH_WDOF_INIT_HF_ERROR_RESET], NULL);
		break;
	case WDOG_NOT_INIT_ERROR:
		system_handler_process_event(sh_event_info[SH_WDOG_NOT_INIT_HF_ERROR_RESET], NULL);
		break;
	case MANU_NOT_INIT_ERROR:
		system_handler_process_event(sh_event_info[SH_MANU_NOT_INIT_HF_ERROR_RESET], NULL);
		break;
	case DEBUG_SHELL_NOT_INIT_ERROR:
		system_handler_process_event(sh_event_info[SH_DEBUG_SHELL_NOT_INIT_HF_ERROR_RESET], NULL);
		break;
	case UNKNOWN_SWITCH_CASE_ERROR:
		system_handler_process_event(sh_event_info[SH_SWITCH_CASE_HF_ERROR_RESET], NULL);
		break;
	case QUEUE_CREATE_ERROR:
		system_handler_process_event(sh_event_info[SH_QUEUE_CREATE_HF_ERROR_RESET], NULL);
		break;
	case EVENT_CREATE_ERROR:
		system_handler_process_event(sh_event_info[SH_EVENT_CREATE_HF_ERROR_RESET], NULL);
		break;
	case UNKNOWN_EVENT_ERROR:
		system_handler_process_event(sh_event_info[SH_UNKNOWN_EVENT_HF_ERROR_RESET], NULL);
		break;
	case MEM_MAN_HASDFAULT_MLSPERR:
		system_handler_process_event(sh_event_info[SH_LAZY_STACKING_MPU_HF_ERROR_RESET], NULL);
		break;
	case MEM_MAN_HARDFAULT_MSTKERR:
		system_handler_process_event(sh_event_info[SH_HEAP_PUSH_HF_ERROR_RESET], NULL);
		break;
	case MEM_MAN_HARDFAULT_MUNSTKERR:
		system_handler_process_event(sh_event_info[SH_HEAP_UNSTACK_HF_ERROR_RESET], NULL);
		break;
	case MEM_MAN_HARDFAULT_DACCVIOL:
		system_handler_process_event(sh_event_info[SH_DATA_ACCSESS_HF_ERROR_RESET], NULL);
		break;
	case MEM_MAN_HARDFAULT_IACCVIOL:
		system_handler_process_event(sh_event_info[SH_INST_ACCSESS_HF_ERROR_RESET], NULL);
		break;
	case BUS_HARDFAULT_LSPERR:
		system_handler_process_event(sh_event_info[SH_LAZY_STACKING_BUS_HF_ERROR_RESET], NULL);
		break;
	case BUS_HARDFAULT_STKERR:
		system_handler_process_event(sh_event_info[SH_BUS_HEAP_STACK_HF_ERROR_RESET], NULL);
		break;
	case BUS_HARDFAULT_UNSTKERR:
		system_handler_process_event(sh_event_info[SH_BUS_HEAP_UNSTACK_HF_ERROR_RESET], NULL);
		break;
	case BUS_HARDFAULT_IMPRECISERR:
		system_handler_process_event(sh_event_info[SH_IMP_DATA_BUS_HF_ERROR_RESET], NULL);
		break;
	case BUS_HARDFAULT_PRECISERR:
		system_handler_process_event(sh_event_info[SH_PRE_DATA_BUS_HF_ERROR_RESET], NULL);
		break;
	case BUS_HARDFAULT_IBUSERR:
		system_handler_process_event(sh_event_info[SH_INST_BUS_HF_ERROR_RESET], NULL);
		break;
	case USAGE_HARDFAULT_DIVBYZERO:
		system_handler_process_event(sh_event_info[SH_DIVIDED_ZERO_HF_ERROR_RESET], NULL);
		break;
	case USAGE_HARDFAULT_UNALIGNED:
		system_handler_process_event(sh_event_info[SH_UNALIGNED_ACCESS_HF_ERROR_RESET], NULL);
		break;
	case USAGE_HARDFAULT_NOCP:
		system_handler_process_event(sh_event_info[SH_NO_COPROC_HF_ERROR_RESET], NULL);
		break;
	case USAGE_HARDFAULT_INVPC:
		system_handler_process_event(sh_event_info[SH_INVALID_PC_HF_ERROR_RESET], NULL);
		break;
	case USAGE_HARDFAULT_INVSTATE:
		system_handler_process_event(sh_event_info[SH_INVALID_STATE_HF_ERROR_RESET], NULL);
		break;
	case USAGE_HARDFAULT_UNDEFINSTR:
		system_handler_process_event(sh_event_info[SH_UNDEF_INST_HF_ERROR_RESET], NULL);
		break;
	case ERROR_SENDING_MQTT:
		system_handler_process_event(sh_event_info[SH_MQTT_FAILED_ERROR_RESET], NULL);
		break;
	case ERROR_FLASH_ERASE_FAILED:
		system_handler_process_event(sh_event_info[SH_FLASH_ERASE_ERROR_RESET], NULL);
		break;
	case CMD_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_CMD_BLOCKED_ERROR_RESET], NULL);
		break;
	case ENERGY_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_ENERGY_BLOCKED_ERROR_RESET], NULL);
		break;
	case FW_UPDATE_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_FW_UPDATE_BLOCKED_ERROR_RESET], NULL);
		break;
	case LED_HANDLER_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_LED_HANDLER_BLOCKED_ERROR_RESET], NULL);
		break;
	case FAST_VARS_STREAMING_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_FAST_VARS_STREAM_BLOCKED_ERROR_RESET], NULL);
		break;
	case SLOW_VARS_STREAMING_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_SLOW_VARS_STREAM_BLOCKED_ERROR_RESET], NULL);
		break;
	case FAST_VARS_READ_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_FAST_VARS_READ_BLOCKED_ERROR_RESET], NULL);
		break;
	case SLOW_VARS_READ_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_SLOW_VARS_READ_BLOCKED_ERROR_RESET], NULL);
		break;
	case TIME_SYNC_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_TIME_SYNC_BLOCKED_ERROR_RESET], NULL);
		break;
	case WATCHDOG_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_WATCHDOG_BLOCKED_ERROR_RESET], NULL);
		break;
	case SYSTEM_BLOCKED_ERROR:
		system_handler_process_event(sh_event_info[SH_SYSTEM_BLOCKED_ERROR_RESET], NULL);
		break;
	default:
		/* Hardfault error not detected */
		break;
	}
}

static void system_handler_change_state(system_states_t _new_state, uint32_t _state_events, char* _dbg_message)
{
	system_state = _new_state;
	state_events = _state_events;
	
	absl_event_clear_events(&system_events, SYSTEM_EVENTS_TO_CLEAR);

	absl_debug_printf(_dbg_message);
}

static void system_handler_get_and_proccess_event(void)
{
	event_info_t 	system_event;
	uint32_t 		event_to_give = 0;

	bool error = false;

	while(ABSL_QUEUE_RV_OK == absl_queue_receive(&events_info_queue, (void*)&system_event, ABSL_QUEUE_NO_DELAY))
	{
		event_to_give |= system_handler_process_event(system_event, &error);
	}

	if(true == error)
	{
		absl_queue_send(&send_states_queue, (void*)&send_data.state_change_data, ABSL_QUEUE_MAX_DELAY);
		system_handler_evaluate_error(send_data.state_change_data);
	}

	if(event_to_give != 0)
	{
		absl_event_set(&cmd_events, event_to_give);
	}
}

static uint32_t system_handler_process_event(event_info_t _system_event, bool* _error_detected)
{
	uint32_t event_to_give = 0;

	if(true == system_connected_to_server)
	{
		event_to_give = system_handler_send_event(_system_event, _error_detected);
	}
	else
	{
		if(no_conn_event_num < MAX_EVENT_NO_CONN)
		{
			detecter_event_no_conn[no_conn_event_num] = _system_event;
			no_conn_event_num++;
		}
		else
		{
			for(uint32_t event_index = 0; event_index < (MAX_EVENT_NO_CONN - 1); event_index++)
			{
				detecter_event_no_conn[event_index]  = detecter_event_no_conn[event_index + 1];
			}
			detecter_event_no_conn[MAX_EVENT_NO_CONN - 1] = _system_event;
			*_error_detected = false;
		}
	}

	return event_to_give;
}

static void system_handler_send_no_conn_events(void)
{
	uint32_t event_to_give = 0;

	bool error = false;

	for(uint32_t event_index = 0; event_index < no_conn_event_num; event_index++)
	{
		event_to_give |= system_handler_send_event(detecter_event_no_conn[event_index], &error);
	}

	if(true == error)
	{
		absl_queue_send(&send_states_queue, (void*)&send_data.state_change_data, ABSL_QUEUE_MAX_DELAY);
		system_handler_evaluate_error(send_data.state_change_data);
	}

	no_conn_event_num = 0;
	absl_event_set(&cmd_events, event_to_give);
}

static uint32_t system_handler_send_event(event_info_t _system_event, bool* _error_detected)
{
	uint32_t event_to_give = 0;
	event_data_to_send_t event_data;

	event_data = event_handler_new_event_code(&_system_event);

	if(EVENT_TYPE_ERROR == event_data.type)
	{
		/* if the alert is an error state will change to failure */
		*_error_detected = true;
		send_data.state_change_data = event_data.data.state_change_data;
		event_to_give = CMD_SEND_STATUS_CHANGE;
	}
	else if(EVENT_TYPE_WARNING == event_data.type || EVENT_TYPE_INFO == event_data.type)
	{
		send_data.alert_data = event_data.data.alert_data;

		absl_queue_send(&send_alets_queue, (void*)&send_data.alert_data, ABSL_QUEUE_MAX_DELAY);
		event_to_give = CMD_SEND_ALERT;
	}
	else
	{
		/* No event or error to give */
	}

	return event_to_give;
}

static void system_handler_evaluate_error(state_change_data_t _state_change)
{
	switch(_state_change.state_type)
	{
		case DEVICE_STATE:
			system_handler_change_state(SYSTEM_ERROR_STATE, SYSTEM_ERROR_STATE_EVENTS, SYSTEM_ERROR_STATE_MSG);
		break;
		case SENSOR_STATE:
			system_handler_state_change_info_send(SERVICE_STATE, SENSOR_ADE9000, SERVICE_RAW, (uint32_t)SERVICE_STATUS_IDLE);
			system_handler_state_change_info_send(SERVICE_STATE, SENSOR_ADE9000, SERVICE_REGISTERS, (uint32_t)SERVICE_STATUS_IDLE);
		break;
		case SERVICE_STATE:
		break;
		default:
		break;
	}
}

static void system_handler_state_change_info_send(state_type_t _state_type, sensors_t _sensor, 
												  services_t _service, uint32_t _new_status)
{
	state_change_data_t 	state_change;

	error_codes = event_handler_get_error_codes(SENSOR_ADE9000, SERVICE_RAW, &error_amount);
	
	state_change.error_codes = error_codes;
	state_change.error_code_amount = error_amount;
	state_change.state_type = _state_type;

	switch(_state_type)
	{
		case DEVICE_STATE:
		state_change.state_info.device_info.status = (device_status_t)_new_status;
		break;
		case SENSOR_STATE:
		state_change.state_info.sensor_info.status = (sensor_status_t)_new_status;
		state_change.state_info.sensor_info.sensor = _sensor;
		break;
		case SERVICE_STATE:
		state_change.state_info.service_info.status = (service_status_t)_new_status;
		state_change.state_info.service_info.sensor = _sensor;
		state_change.state_info.service_info.service = _service;
		break;
		default:
		break;
	}
	
	send_data.state_change_data = state_change;
	absl_queue_send(&send_states_queue, (void*)&send_data.state_change_data, ABSL_QUEUE_MAX_DELAY);
}
