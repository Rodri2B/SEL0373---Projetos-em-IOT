#include <stdio.h> // para funcões de print 
#include <string.h> //para lidar com sitrings
#include "freertos/FreeRTOS.h" //para os delays
#include "freertos/task.h" 
#include "esp_wifi.h" //configuracao do wifi
#include "esp_system.h" // gerencia o sistema da esp32
#include "esp_log.h" //logs no terminal
#include "esp_event.h" // manipular eventos
#include "nvs_flash.h" //memoria nao volatil
#include "esp_netif.h" 
#include "protocol_examples_common.h" //facilita a manipulacao de protoclos(wifi,mqtt etc)
// biblioteca para manipulacao de pilha TCP/IP
#include "lwip/err.h" 
#include "lwip/sys.h" 
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "mqtt_client.h" // funcoes para a conexao MQTT

#define  SSID "Sofy_A53"
#define  SENHA "kgil33560"
#define BLINK_GPIO 2


uint32_t MQTT_conexao = 0; //flag para identificar a conexao do MQTT

static uint8_t s_led_state = 0;
static const char *TAG1 = "wifi";
static const char *TAG = "MQTT";

int retry_num=0;

static void blink_led(void);
// Lidando com os eventos de Wi-fi, verificando o stutus atual da conexao e obtencao do IP
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,void *event_data){
    switch (event_id)
    {
    case WIFI_EVENT_STA_START: //quando o wifi esta tentando conectar
        esp_wifi_connect(); // conecta a rede
        ESP_LOGI(TAG1, "Tentando conectar ao Wi-Fi...\n");
        break;

    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG1, "Wi-Fi coectado\n"); // conectado ao ponto de acesso(AP)
        break;   

    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG1, "Disconectado do Wi-Fi:Reconectando\n");
        esp_wifi_connect();
        /*if(retry_num<5){
            esp_wifi_connect();
            retry_num++;
            printf("Retrying to Connect...\n");
        }*/

        break;

    case IP_EVENT_STA_GOT_IP: //wifi recebeu o ip da rede
        ESP_LOGI(TAG1, "IP recebido\n");

        break;

    default:
        break;
    }
}

// Funcao que sera chamado no app_main para configurar o wifi
void wifi_connection(void){
    esp_netif_init();
    esp_event_loop_create_default(); //cria um loop dos eventos para que sejam manuseados e enviados 
    esp_netif_create_default_wifi_sta(); //incializacao do wifi para o modo estaocao e trata os eventos
    wifi_init_config_t wifi_default = WIFI_INIT_CONFIG_DEFAULT(); // configuracao padrao do wifi
    esp_wifi_init(&wifi_default); // driver inicializado com os valores padrao
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL); // registra o wifi_event para obter qualquer variacao/evento do wifi
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL); // configura eventos Ip
   
   wifi_config_t wifi_param = {
        .sta = {
            .ssid = SSID,
            .password = SENHA,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA); // configura para o modo estacao
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_param); // coloca os parametros
    esp_wifi_start(); //tenta conectar com as configuracoes salvas
}

// configuracoes do MQTT e seus eventos
/*static void mqtt_event_handler(esp_mqtt_event_handle_t event){
    //esp_mqtt_event_handle_t e uma struct que receb a struct "event", a qual contem as variaveis do evento MQTT
    //esp_mqtt_event_handle_t event = event_data; //obtem os dados do evento MQTT
    esp_mqtt_client_handle_t client = event->client; //obtem o conteudo do identificador cliente dentro da struct 
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED: // cliente conectado ao broker
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED"); //exibe o lpg
        esp_mqtt_client_subscribe(client, "teste", 0); // Inscreve-se no tópico MQTT
        esp_mqtt_client_publish(client, "teste", "ola da ESP32.....", 0,0,0);
        MQTT_conexao = 1;
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED"); // Quando desconectado
        MQTT_conexao = 0;
        break;

    case MQTT_EVENT_SUBSCRIBED: //quando se inscreveru com sucesso no topico
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
        break;

    case MQTT_EVENT_UNSUBSCRIBED: // quando cancela a inscricao
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED"); 
        break;
    
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED"); //quando o broker reconhece a publicacao no topico
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR"); // Em caso de erro
        break;
        
    case MQTT_EVENT_ERROR:
    ESP_LOGE(TAG, "Erro MQTT detectado!");

    if (event->error_handle == NULL) {
        ESP_LOGE(TAG, "event->error_handle é NULL! Não há detalhes do erro.");
        break;
    }

    switch (event->error_handle->error_type) {
        case MQTT_ERROR_TYPE_TCP_TRANSPORT:
            ESP_LOGE(TAG, "Erro de transporte TCP! Código: %d", event->error_handle->esp_tls_last_esp_err);
            break;

        case MQTT_ERROR_TYPE_CONNECTION_REFUSED:
            ESP_LOGE(TAG, "Conexão recusada! Código: %d", event->error_handle->connect_return_code);
            break;

        default:
            ESP_LOGE(TAG, "Erro desconhecido! Tipo de erro: %d", event->error_handle->error_type);
            break;
    }
    break;


    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }


}
esp_mqtt_client_handle_t client = NULL;
static void mqtt_app_start(void){

    const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = "mqtt://igbt.eesc.usp.br",
    .credentials = {
        .username = "mqtt",
        .authentication.password = "mqtt_123_abc"
    }
};

    esp_mqtt_client_handle_t client =esp_mqtt_client_init(&mqtt_cfg); //envia os parametros do mqtt para inicializar
    if (client == NULL) {
    ESP_LOGE(TAG, "Falha ao inicializar o cliente MQTT!");
    return;
    }
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);  //coloca o manipulador definido acima
    esp_mqtt_client_start(client); // inicia o cliente mqtt com as configuracoes
}

void Publisher_Task(void *params) // publica a mensagem no topico a cada 15 segundos
{
  while (true)
  {
    if(MQTT_conexao)
    {
        esp_mqtt_client_publish(client, "teste", "Conectado no PC!", 0, 0, 0);
        vTaskDelay(15000 / portTICK_PERIOD_MS);
    }
  }
}
void app_main(void){
    nvs_flash_init(); // mantem guardada as configuracoes como wifi
    wifi_initia(); //chama a funcao para a incializacao do wifi
    vTaskDelay(10000 /portTICK_PERIOD_MS); // delay para esperar a conexao
    mqtt_app_start(); // inicia a conexao mqtt
    //xTaskCreate(Publisher_Task, "Publisher_Task", 1024 * 5, NULL, 5, NULL);
}*/

// configuracoes do MQTT e seus eventos
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
    //esp_mqtt_event_handle_t e uma struct que receb a struct "event", a qual contem as variaveis do evento MQTT
    //esp_mqtt_event_handle_t event = event_data; //obtem os dados do evento MQTT
{
    esp_mqtt_client_handle_t client = event->client; //obtem o conteudo do identificador cliente dentro da struct
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED: // cliente conectado ao broker
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED"); //exibe o log
        esp_mqtt_client_subscribe(client, "teste", 0); //Inscreve-se no tópico MQTT
        esp_mqtt_client_publish(client, "teste", "Oi da ESP32 .........", 0, 1, 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED: //quando se inscreveru com sucesso no topico
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED: // quando cancela a inscricao
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED: //quando o broker reconhece a publicacao no topico
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA: //Qiuando o cliente recebe a mensagem publicada
    // o event (struct com os dados do evento) guarda o id, nome do topico, dado e tamanho do dado
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("\nTOPIC=%.*s\r\n", event->topic_len, event->topic);//nome do topico
        printf("DATA=%.*s\r\n", event->data_len, event->data); //nomr do dado (mensagem)
        if (strncmp(event->data, "led", event->data_len) == 0){
            printf ("Mensagem válida\r\n");
            ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");// exibe o log com o status 
            if (s_led_state==true){ //verifica o status do led
            //publica mensagens
                esp_mqtt_client_publish(client, "teste", "Led ligado .........", 0, 1, 0);
            }
            else{
                esp_mqtt_client_publish(client, "teste", "Led desligado .........", 0, 1, 0);
            }
            blink_led();
        /* Trocando o estado do led */
            s_led_state = !s_led_state;//a cada publicação troca o estado do led
            vTaskDelay(1000/ portTICK_PERIOD_MS);
        }
        break;
    case MQTT_EVENT_ERROR: //em caso de erro
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Evento dispachado do loop=%s, event_id=%ld", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = "mqtt://igbt.eesc.usp.br",
    .credentials = {
        .username = "mqtt",
        .authentication.password = "mqtt_123_abc"
    }
};
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    nvs_flash_init(); //guarda as config do wifi, como 
    wifi_connection(); //config do 
    configure_led();//inicia a config do led

    vTaskDelay(2000 / portTICK_PERIOD_MS); // esperea um pouco ate configurar o wifi
    printf("WIFI foi iniciado ...........\n");

    
    mqtt_app_start();
}






