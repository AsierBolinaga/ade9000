# VArs read

 Hilo encargado de obtener los datos correspondientes al servicio y mandarselos al hilo de strreaming.

 ![](../../doc/vars%20read%20class%20diagram.png)  

## Configuración del hilo varr read

El hilo vars read es configurado utilizando al siguiente estructura:

~~~
typedef struct vars_read_thread_config
{
	pl_event_t*			 		vars_events;
	pl_event_t*					stream_event;
	uint32_t					data_to_send_flag;
	pl_queue_t*			 		read_data_send_queue;
	bool 						(*vars_read_cb)(void* _vars_config, void* _vars_value, event_info_t* _event_info);
	void 						(*vars_config_cb)(void* _vars_config);
	void*						read_vars;
	void* 						vars_config;
	bool				 		vars_read_initialized;
}vars_read_thread_config_t;
~~~

- **vars_events**: Grupo de eventos al que atiende el hilo.
- **stream_event**: Grupo de eventos utilizado para mandar eventos al hilo de streaming.
- **data_to_send_flag**: Evento que indica al hilo de streaming que tiened datos para enviar.
- **read_data_send_queue**: Queue a donde madnara los datos leidos.
- **vars_read_cb**: Callback para leer los datos del sensor y servicio correspondiente.
- **vars_config_cb**: Callback para obtener la configuración del servicio.
- **read_vars**: Puntero a las variables leidas.
- **vars_config**: Puntero a la configuracion obtenida.
- **vars_read_initialized**: Inicializarlo siempre a false. Indica si el hilo ha sido inicializado correctamente o no. De esta forma el hilo se ira a error en caso de ejecutarlo antes de inicializarlo.


## Estados del hilo energy

El hilo espera ha eventos para ejecutarse y dependiendo del estado en el que este el sistema, el hilo energy se ejecutara en un estado, y hara lo necesario dependiendo del evento:

 ![](../../doc/images/Energy_states.png) 

- **ENERGY_TASK_IDLE**: El hilo no hace nada, solo atiende a eventos de lectura de variables de energia en el caso de que ocurran, para resetar lo necesario y poder empezar a leer correctamente cuando sea necesario.
  
   ![](../../doc/images/energy_idle_state.png) 




