// /* UART Echo Example
//    This example code is in the Public Domain (or CC0 licensed, at your option.)
//    Unless required by applicable law or agreed to in writing, this
//    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//    CONDITIONS OF ANY KIND, either express or implied.
// */
// #include <stdio.h>
// #include "driver/uart.h"

// /**
//  * This is an example which echos any data it receives on UART0 back to the sender,
//  * with hardware flow control turned off. It does not use UART driver event queue.
//  *
//  * - Port: UART0
//  * - Receive (Rx) buffer: on
//  * - Transmit (Tx) buffer: off
//  * - Flow control: off
//  * - Event queue: off
//  * - Pin assignment: use the default pins rather than making changes
//  */

// #define EX_UART_NUM UART_NUM_0
// #define ECHO_TEST_TXD  (UART_PIN_NO_CHANGE)
// #define ECHO_TEST_RXD  (UART_PIN_NO_CHANGE)
// #define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
// #define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

// #define BUF_SIZE (1024)

// void app_main(void)
// {
//     /* Configure parameters of the UART driver,
//      * communication pins and install the driver */
//     uart_config_t uart_config = {
//         .baud_rate = 115200,
//         .data_bits = UART_DATA_8_BITS,
//         .parity    = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//         .source_clk = UART_SCLK_APB,
//     };
//     uart_param_config(EX_UART_NUM, &uart_config);
//     uart_set_pin(EX_UART_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
//     uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    
//     // Configure a temporary buffer for the incoming data
//     uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

//     // Display prompt
//     printf("Enter value: \n");

//     while (1) {
//         // Read data from the UART
//         int len = uart_read_bytes(EX_UART_NUM, data, BUF_SIZE, 20 / portTICK_RATE_MS);
//         // Write data back to the UART
//         if (len > 0) {
//           printf("Read: %c \n\nEnter value: \n", *data);
//         }
//     }
// }
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define BLINK_GPIO 13

void app_main()
{
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    //char str[20];
    int num = 0;
    int counter = 0;
    int led = 0;
        
    while(1) {
      char input[10];
      gets(input);
      if (input[0] == 's') {
        counter++;
        if (counter % 3 == 0) {
        printf("Switched to Toggle LED Mode\n");
        }
        else if (counter % 3 == 1) {
        printf("Switched to Echo Input Mode\n");
        }
        else if (counter % 3 == 2) {
        printf("Switched to Decimal to Hexadecimal Mode\n");
        }
    }

    if (counter%3 == 0) {
      // toggle LED mode
      printf("Read: ");
      if (input[0] != '\0') {
       printf("%s\n", input);
      }
      if (input[0] == 't') {
        // add the blinking here
        if (led == 0) {
          gpio_set_level(BLINK_GPIO,1);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          led = 1;
        }
        else if (led == 1) {
          gpio_set_level(BLINK_GPIO,0);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          led = 0;
        }
      }
    }

    if (counter%3 == 1) {
      printf("echo: ");
      // gets()
      if (input[0] != '\0') {
       printf("%s\n", input);
      }
    }

    if (counter%3 == 2) {
      printf("Enter an integer:\n");
      //gets();
      int temp_hold = atoi(input);
      //num = DtoHex(temp_hold);
      num = temp_hold;
      char hex_num[100];
      char out[100];
      int i = 0;
      
      if (input[0]!='\0')
      {
        while (num!=0) {
          
          int temp = 0;
          temp = num%16;
          if (temp<10)
          {
            hex_num[i] = temp+48;
            i++;
          } else {
            hex_num[i] = temp+ 55;
            i++;
          }
          num = num/16;
      }
      }
        // int begin, count, end = 0;
        // while (hex_num[begin]!='\0') 
        //   count++;
        // end = count-1;
        // for (begin; begin<count;begin++) {
        //   out[begin]=hex_num[end];
        //   end--;
        // }
        // out[begin]='\0';
        
      // for (int j = i-1; j>=0; j--)
      //   cout<< hex_num[j];
      //strrev(hex_num);
      printf("Hex: %s\n", hex_num);
    }
    
      vTaskDelay(50 / portTICK_RATE_MS);
    }
}
