#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_netif.h>
#include <esp_eth.h>
#include <esp_mac.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_check.h>

static const char *TAG = "ruuvi-gw-ethernet";

static EventGroupHandle_t s_ruuvi_gw_ethernet_event_group;

/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        xEventGroupSetBits(s_ruuvi_gw_ethernet_event_group, BIT1);
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    // got IP so set bit in ethernet event group
    xEventGroupSetBits(s_ruuvi_gw_ethernet_event_group, BIT0);
}

#if CONFIG_RUUVI_GW_USE_ETHERNET
static esp_eth_handle_t eth_init_internal(esp_eth_mac_t **mac_out, esp_eth_phy_t **phy_out)
{
    esp_eth_handle_t ret = NULL;

    // Init common MAC and PHY configs to default
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

    // Update PHY config based on board specific configuration
    phy_config.phy_addr = CONFIG_RUUVI_GW_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = CONFIG_RUUVI_GW_ETH_PHY_RST_GPIO;

    // Init vendor specific MAC config to default
    eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();

    // Update vendor specific MAC config based on board configuration
    esp32_emac_config.smi_gpio.mdc_num = CONFIG_RUUVI_GW_ETH_MDC_GPIO;
    esp32_emac_config.smi_gpio.mdio_num = CONFIG_RUUVI_GW_ETH_MDIO_GPIO;

    // Create new ESP32 Ethernet MAC instance
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config);

    // Create new PHY instance based on board configuration
    esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);

    // Init Ethernet driver to default and install it
    esp_eth_handle_t eth_handle = NULL;
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_GOTO_ON_FALSE(esp_eth_driver_install(&config, &eth_handle) == ESP_OK, NULL,
                        err, TAG, "Ethernet driver install failed");

    if (mac_out != NULL) {
        *mac_out = mac;
    }
    if (phy_out != NULL) {
        *phy_out = phy;
    }

    return eth_handle;

err:
    if (eth_handle != NULL) {
        esp_eth_driver_uninstall(eth_handle);
    }
    if (mac != NULL) {
        mac->del(mac);
    }
    if (phy != NULL) {
        phy->del(phy);
    }

    return ret;
}
#endif // CONFIG_RUUVI_GW_USE_ETHERNET

#if CONFIG_RUUVI_GW_USE_ETHERNET
esp_err_t ruuvi_gw_ethernet_init()
{
    esp_err_t ret = ESP_OK;
    esp_eth_handle_t *eth_handle = NULL;

    s_ruuvi_gw_ethernet_event_group = xEventGroupCreate();

    eth_handle = malloc(sizeof(esp_eth_handle_t));
    ESP_GOTO_ON_FALSE(eth_handle != NULL, ESP_ERR_NO_MEM, err, TAG, "no memory");

    eth_handle = eth_init_internal(NULL, NULL);
    ESP_GOTO_ON_FALSE(eth_handle, ESP_FAIL, err, TAG, "internal Ethernet init failed");

    // Initialize TCP/IP network interface aka the esp-netif (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());

    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used and you don't need to modify
    // default esp-netif configuration parameters.
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);

    // Attach Ethernet driver to TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    ESP_ERROR_CHECK(esp_eth_start(eth_handle));

    // wait for interface to come up
    EventBits_t bits = xEventGroupWaitBits(s_ruuvi_gw_ethernet_event_group,
            BIT0 | BIT1,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    vEventGroupDelete(s_ruuvi_gw_ethernet_event_group);

    if (bits & BIT0) {
        ESP_LOGI(TAG, "Connected to Ethernet");
    }
    else if (bits & BIT1) {
        ESP_LOGI(TAG, "Ethernet link failed");
        return ESP_FAIL;
    }
    else {
        ESP_LOGE(TAG, "Unexpected event");
        return ESP_FAIL;
    }

    return ret;

err:
    free(eth_handle);
    return ret;
}
#else
esp_err_t ruuvi_gw_ethernet_init()
{
    return ESP_OK;
}
#endif // CONFIG_RUUVI_GW_USE_ETHERNET
