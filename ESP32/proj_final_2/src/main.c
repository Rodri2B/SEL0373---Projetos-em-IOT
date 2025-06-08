#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

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

#define GPIO_DATA   GPIO_NUM_18
#define GPIO_SCLK   GPIO_NUM_15

#define  SSID "mohamad"
#define  SENHA "kolkhara1234"

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
esp_mqtt_client_handle_t client;

void send_task(int ID, bool flag);

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

void separarNumeros(const char *entrada, int *num1, int *num2, int data_len) {
    // Copia a string de entrada para uma variável local para não alterar a original
    ESP_LOGI(TAG, "Tamanho  %d", data_len);
    char buffer[data_len+1];
    strncpy(buffer, entrada, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';  // garante terminação

    // Usa strtok para separar pela vírgula
    char *parte1 = strtok(buffer, ",");
    char *parte2 = strtok(NULL, ",");

    if(parte1 != NULL && parte2 != NULL) {
        *num1 = atoi(parte1); // converte para inteiro
        *num2 = atoi(parte2);
    } else {
        // Caso a string não esteja no formato esperado
        *num1 = 0;
        *num2 = 0;
    }
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

        // funcao para separar dados
        int num1, num2;
        int data_len = event->data_len;
        separarNumeros(event->data, &num1, &num2, data_len);
        bool flag = num2;
        ESP_LOGI(TAG, "event data: %s", event->data);
        ESP_LOGI(TAG, "num1: %d num2: %d", num1, num2);
        // int ID = num1;
        // bool booleano = num2;
        send_task(num1,flag);
        // echo
        // msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC_PUB, event->data, event->data_len, 2, 0);

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

        break;
    }
}

void send_task_confirmacao() {

                twai_message_t message = {
                    .identifier = 0x103,  // Set message ID
                    .data_length_code = 8,  // Set data length
                    .data = {1,0,0,0,0,0,0,0}  // Data to send
                };

                esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(500));
                if (err == ESP_OK) {

                    ESP_LOGI(TAG, "Message sent successfully");
                    //ESP_LOGI(TAG,"Erro %d",err);

}
}

unsigned int weight =0;

void receive_task(void *arg) {
    while (1) {
        char buf[50];
        twai_message_t message;
        esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(50));
        if (err == ESP_OK) {
            int id =         ((uint32_t)message.data[3] << 24) |
                             ((uint32_t)message.data[2] << 16) |
                             ((uint32_t)message.data[1] << 8)  |
                             ((uint32_t)message.data[0]);

            int weight =     ((uint32_t)message.data[7] << 24) |
                             ((uint32_t)message.data[6] << 16) |
                             ((uint32_t)message.data[5] << 8)  |
                             ((uint32_t)message.data[4]);
        sprintf(buf,"%d,%d",id,weight);
        int len =  strlen(buf);
        int msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC_PUB, buf, len, 2, 0);
        	ESP_LOGI(TAG,"ID: %d Valor: %d", id, weight);
            // send_task_confirmacao();
        
        }
        /*else if (err == ESP_ERR_TIMEOUT) {
            ESP_LOGW(TAG, "Reception timed out");
        } else {
            ESP_LOGE(TAG, "Message reception failed: %s", esp_err_to_name(err));
        }
        */
    }
}

void send_task(int ID, bool flag) {
    	uint8_t buf[8];
        buf[0] = (ID >> 0)  & 0xFF;  // byte menos significativo
        buf[1] = (ID >> 8)  & 0xFF;
        buf[2] = (ID >> 16) & 0xFF;
        buf[3] = (ID >> 24) & 0xFF;  // byte mais significativo
        ESP_LOGI(TAG, "Message sent: Data=[%02X %02X %02X %02X]",
                 buf[0], buf[1], buf[2], buf[3]);
        twai_message_t message = {
            .identifier = 0x102,  // Set message ID
            .data_length_code = 5,  // Set data length
            .data = {buf[0], buf[1], buf[2], buf[3], flag, 0x00, 0x00, 0x00}  // Data to send
        };

        esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(1000));
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Message sent successfully");
        } else {
            ESP_LOGE(TAG, "Message transmission failed: %s", esp_err_to_name(err));
        }
}

void app_main() {
    // Configure TWAI driver
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();  // Set to 1 Mbps
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();  // Accept all messages

    // Install TWAI driver
    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver installed successfully");
    } else {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    // Start TWAI driver
    err = twai_start();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    // Create tasks
   xTaskCreate(receive_task, "receive_task", 2048, NULL, 5, NULL);

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
    client = esp_mqtt_client_init(&mqtt_cfg);

    // Registra o manipulador de eventos para o cliente
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    // Inicia o cliente MQTT
    esp_mqtt_client_start(client);
}