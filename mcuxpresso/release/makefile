################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include source/sbl/scripts/subdir.mk
-include source/platform/imxrt1062/xip/subdir.mk
-include source/platform/imxrt1062/utilities/subdir.mk
-include source/platform/imxrt1062/startup/subdir.mk
-include source/platform/imxrt1062/flash/mflash/subdir.mk
-include source/platform/imxrt1062/drivers/freertos/subdir.mk
-include source/platform/imxrt1062/drivers/subdir.mk
-include source/platform/imxrt1062/device/subdir.mk
-include source/platform/imxrt1062/component/uart/subdir.mk
-include source/platform/imxrt1062/component/silicon_id/subdir.mk
-include source/platform/imxrt1062/component/serial_manager/subdir.mk
-include source/platform/imxrt1062/component/phy/subdir.mk
-include source/platform/imxrt1062/component/lists/subdir.mk
-include source/platform/imxrt1062/component/flashiap/subdir.mk
-include source/platform/imxrt1062/CMSIS/NN/Source/SoftmaxFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/NN/Source/SVDFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/NN/Source/ReshapeFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/NN/Source/PoolingFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/NN/Source/NNSupportFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/NN/Source/FullyConnectedFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/NN/Source/ConvolutionFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/NN/Source/ConcatenationFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/NN/Source/BasicMathFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/NN/Source/ActivationFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/TransformFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/SupportFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/StatisticsFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/SVMFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/QuaternionMathFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/MatrixFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/InterpolationFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/FilteringFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/FastMathFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/DistanceFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/ControllerFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/ComplexMathFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/CommonTables/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/BayesFunctions/subdir.mk
-include source/platform/imxrt1062/CMSIS/DSP/Source/BasicMathFunctions/subdir.mk
-include source/platform/OS/sfw/subdir.mk
-include source/platform/OS/lwip/src/netif/ppp/polarssl/subdir.mk
-include source/platform/OS/lwip/src/netif/ppp/subdir.mk
-include source/platform/OS/lwip/src/netif/subdir.mk
-include source/platform/OS/lwip/src/core/ipv6/subdir.mk
-include source/platform/OS/lwip/src/core/ipv4/subdir.mk
-include source/platform/OS/lwip/src/core/subdir.mk
-include source/platform/OS/lwip/src/apps/sntp/subdir.mk
-include source/platform/OS/lwip/src/apps/mqtt/subdir.mk
-include source/platform/OS/lwip/src/apps/mdns/subdir.mk
-include source/platform/OS/lwip/src/api/subdir.mk
-include source/platform/OS/lwip/port/subdir.mk
-include source/platform/OS/lwip/contrib/apps/ping/subdir.mk
-include source/platform/OS/freertos/freertos-kernel/portable/MemMang/subdir.mk
-include source/platform/OS/freertos/freertos-kernel/portable/GCC/ARM_CM4F/subdir.mk
-include source/platform/OS/freertos/freertos-kernel/subdir.mk
-include source/data_extractor/watchdog/subdir.mk
-include source/data_extractor/vars_read/subdir.mk
-include source/data_extractor/timesync/subdir.mk
-include source/data_extractor/streaming/subdir.mk
-include source/data_extractor/sensors/ade9000/subdir.mk
-include source/data_extractor/protocol/mqtt/subdir.mk
-include source/data_extractor/protocol/json/subdir.mk
-include source/data_extractor/manufacturing/subdir.mk
-include source/data_extractor/led_handler/subdir.mk
-include source/data_extractor/fw_update/sfw/subdir.mk
-include source/data_extractor/fw_update/subdir.mk
-include source/data_extractor/event_handler/subdir.mk
-include source/data_extractor/energy/subdir.mk
-include source/data_extractor/debug/subdir.mk
-include source/data_extractor/cmd/subdir.mk
-include source/data_extractor/subdir.mk
-include source/components/subdir.mk
-include source/cJSON/subdir.mk
-include source/board/subdir.mk
-include source/abstraction_layer/subdir.mk
-include source/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := ade9000_data_extractor
BUILD_ARTIFACT_EXTENSION := axf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all:
	+@$(MAKE) --no-print-directory pre-build && $(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: ade9000_data_extractor.axf

# Tool invocations
ade9000_data_extractor.axf: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -Xlinker -Map="ade9000_data_extractor.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -Xlinker --sort-section=alignment -Xlinker --cref -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -T ../linkscripts/energy_freerun.ld -o "ade9000_data_extractor.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) ade9000_data_extractor.axf
	-@echo ' '

pre-build:
	-export ProjDirTopLevel="/home/abolinaga/OneDrive/Shared Folder/repo/ade9000/mcuxpresso/../";
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-export ProjDirTopLevel="/home/abolinaga/OneDrive/Shared Folder/repo/ade9000/mcuxpresso/../" ; export ProjDirPath="/home/abolinaga/OneDrive/Shared Folder/repo/ade9000/mcuxpresso" ; export BuildArtifactFileName="ade9000_data_extractor.axf" ; export BuildArtifactFileBaseName="ade9000_data_extractor" ; export TargetChip="MIMXRT1062xxxxB" ; ../scripts/release/post-build
	-@echo ' '

.PHONY: all clean dependents main-build pre-build post-build

-include ../makefile.targets
