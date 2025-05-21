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
//#include "protocol_examples_common.h" //facilita a manipulacao de protoclos(wifi,mqtt etc)
// biblioteca para manipulacao de pilha TCP/IP
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "mqtt_client.h" // funcoes para a conexao MQTT

// #define  SSID "LabMicros"
// #define  SENHA "seluspeesc@"
#define  SSID "Redmi_Igor"
#define  SENHA "35516635"

// Defina o endereço do broker MQTT
#define BROKER_URI "mqtt://igbt.eesc.usp.br"

// Tópico que será utilizado
#define MQTT_TOPIC_PUB "vaquinha/echo"
#define MQTT_TOPIC_SUB "vaquinha"

// Defina as credenciais (caso necessário)
#define MQTT_USERNAME "mqtt"
#define MQTT_PASSWORD "mqtt_123_abc"

// Defina um identificador para o cliente
static const char *TAG1 = "WIFI";
static const char *TAG = "MQTT";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}


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



// Função para tratar eventos de MQTT (conexão, mensagens, erros, etc.)
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, MQTT_TOPIC_SUB, 1); // at most once
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        // echo
        msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC_PUB, event->data, event->data_len, 2, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_ERROR:
        
        ESP_LOGI(TAG, "Erro no MQTT.");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        esp_mqtt_client_publish
// Função principal para inicializar e configurar o cliente MQTT
void app_main() {
    
    // Inicializa NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    wifi_connection(); //config do 
    vTaskDelay(5000 / portTICK_PERIOD_MS); // esperea um pouco ate configurar o wifi
    printf("PASSOU AQUI 1");

    // Configuração do cliente MQTT
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = BROKER_URI,  // Definindo o endereço do broker
        .credentials = {
            .username = MQTT_USERNAME,  // Definindo o username
            .authentication.password = MQTT_PASSWORD,  // Definindo a senha para autenticação
        },
    };

    // Inicializa o cliente MQTT
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    // Registra o manipulador de eventos para o cliente
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    // Inicia o cliente MQTT
    esp_mqtt_client_start(client);

    // Publica uma mensagem no tópico 'node' assim que a conexão for estabelecida
    //esp_mqtt_client_publish(client, MQTT_TOPIC, "Mensagem do ESP32!", 0, 1, 0);
    printf("PASSOU AQUI 2");
}