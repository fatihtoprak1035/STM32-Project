
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "liquidcrystal_i2c.h"

ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;

// Pin Tanımlamaları
#define TRIG_PIN GPIO_PIN_0
#define ECHO_PIN GPIO_PIN_1
#define BUZZER_PIN GPIO_PIN_8
#define RGB_R_PIN GPIO_PIN_3
#define RGB_G_PIN GPIO_PIN_4
#define RGB_B_PIN GPIO_PIN_10
#define LED_PIN GPIO_PIN_10  // PA10 için LED pin tanımlaması

// Motor pin tanımlamaları
#define MOTOR_IN1_PIN GPIO_PIN_5  // PB5
#define MOTOR_IN2_PIN GPIO_PIN_9  // PA9

// Buton ve Potansiyometre Pin Tanımlamaları
#define BUTTON_PIN GPIO_PIN_4  // PA4 için buton pin tanımlaması
#define POT_PIN GPIO_PIN_5     // PA5 için potansiyometre (ADC1_IN5)
// Mesafe sınırları (cm cinsinden)
#define NEAR 10   // 10 cm yakın mesafe
#define FAR 100    // 100 cm'den uzak mesafe

int manual_mode = 0; // 0: Otomatik, 1: Manuel

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
void Error_Handler(void);
void delay_us(uint32_t us);
void init_DWT(void);
void play_tone(uint32_t frequency);
void motor_control(uint8_t direction);
uint32_t read_pulse_duration(void);
uint32_t read_pot_value(void);


int main(void)
{


  /* Configure the system clock */
   SystemClock_Config();
   MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  // HAL başlatma
   HAL_Init();

   MX_GPIO_Init();
   MX_I2C1_Init();

   HD44780_Init(2);  // 2 satırlı bir LCD ekran kullanıyoruz

   // LCD ekranı temizle
   HD44780_Clear();
   // LCD başlatma

   HAL_Delay(1000);  // LCD başlatma için bir saniye gecikme ekle


   // DWT başlatma (mikro saniye ölçüm için)
   init_DWT();

   uint32_t distance = 0;

   while (1)
     {
       // Buton durumunu kontrol et
       if (HAL_GPIO_ReadPin(GPIOA, BUTTON_PIN) == GPIO_PIN_RESET)
       {
         manual_mode = 1; // Butona basıldığında manuel moda geç
         // Manuel modda potansiyometreyi sürekli olarak kontrol edin
               HD44780_Clear();
               HD44780_SetCursor(0, 0);
               HD44780_PrintStr("MANUEL MODE");

               while (HAL_GPIO_ReadPin(GPIOA, BUTTON_PIN) == GPIO_PIN_RESET) // Buton basılı kaldığı sürece
               {
                   uint32_t pot_value = read_pot_value(); // Potansiyometre değerini oku


                   if (pot_value > 2000) // Potansiyometre değeri belli bir seviyeyi geçtiyse
                   {   HD44780_SetCursor(0, 1);
                       HD44780_PrintStr("MOTOR:BACKWARD");
                       motor_control(0); // Motoru geri döndür
                   }
                   else
                   {
                	   HD44780_SetCursor(0, 1);
                       HD44780_PrintStr("MOTOR:FORWARD");
                       motor_control(1); // Motoru ileri döndür
                   }

                   HAL_Delay(100); // Gerekirse kısa bir gecikme ekleyin
               }
       }
       else
       {
         manual_mode = 0; // Butona basılmadığında otomatik modda kal
         // Trig pini aktif et (10 us pulse)
           HAL_GPIO_WritePin(GPIOA, TRIG_PIN, GPIO_PIN_SET);
           delay_us(10);
           HAL_GPIO_WritePin(GPIOA, TRIG_PIN, GPIO_PIN_RESET);

           // Echo pini okuma
           uint32_t pulse_duration = read_pulse_duration();
           distance = (pulse_duration * 0.0343) / 2; // Mesafe hesaplama (cm)

           // Mesafeye göre RGB ve Buzzer kontrolü
           if (distance <= NEAR)
           {
             // LCD'yi temizle
             HD44780_Clear();
             // LCD'ye yaz
             HD44780_SetCursor(0, 0);  // 1. satır, 0. sütun
             HD44780_PrintStr("DIST:NEAR");
             HD44780_SetCursor(0, 1);
             HD44780_PrintStr("MOTOR:FORWARD");


             HAL_Delay(200);

             // Kırmızı LED, Buzzer ON
             HAL_GPIO_WritePin(GPIOB, RGB_R_PIN, GPIO_PIN_SET);
             HAL_GPIO_WritePin(GPIOB, RGB_G_PIN | RGB_B_PIN, GPIO_PIN_RESET);
             play_tone(2000); // 2 kHz, 500 ms

             // LED Yanıp Sönmesi
             HAL_GPIO_WritePin(GPIOA, LED_PIN, GPIO_PIN_SET); // LED aç
             HAL_Delay(100); // LED için kısa bir gecikme
             HAL_GPIO_WritePin(GPIOA, LED_PIN, GPIO_PIN_RESET); // LED kapalı
             HAL_Delay(100); // LED için kısa bir gecikme

             // Motoru bir yöne döndür
             motor_control(1); // YAKIN mesafede motor ileri dönecek
           }
           else
           {
             // LCD'yi temizle
             HD44780_Clear();
             // LCD'ye yaz
             HD44780_SetCursor(0, 0);  // 1. satır, 0. sütun
             HD44780_PrintStr("DIST:FAR");
             HD44780_SetCursor(0, 1);
             HD44780_PrintStr("MOTOR:BACKWARD");

             HAL_Delay(200);

             // Yeşil LED, Buzzer OFF
             HAL_GPIO_WritePin(GPIOB, RGB_G_PIN, GPIO_PIN_SET);
             HAL_GPIO_WritePin(GPIOB, RGB_R_PIN | RGB_B_PIN, GPIO_PIN_RESET);
             HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_RESET);  // Buzzer kapalı


             // Motoru diğer yöne döndür
             motor_control(0); // UZAK mesafede motor geri dönecek
           }

       }
       // Kısa bir gecikme
       HAL_Delay(100);
     }
   }




void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

}


static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Trig_Pin|Buzzer_Pin|IN2_Pin|LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RGB_B_Pin|RGB_R_Pin|RGB_G_Pin|IN1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Trig_Pin Buzzer_Pin IN2_Pin LED_Pin */
  GPIO_InitStruct.Pin = Trig_Pin|Buzzer_Pin|IN2_Pin|LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : Echo_Pin */
  GPIO_InitStruct.Pin = Echo_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Echo_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Button_Pin */
  GPIO_InitStruct.Pin = Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RGB_B_Pin RGB_R_Pin RGB_G_Pin IN1_Pin */
  GPIO_InitStruct.Pin = RGB_B_Pin|RGB_R_Pin|RGB_G_Pin|IN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}
void delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (HAL_RCC_GetHCLKFreq() / 1000000);

  while ((DWT->CYCCNT - start) < ticks);
}

// DWT başlatma fonksiyonu
void init_DWT(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}
// Motor kontrol fonksiyonu
void motor_control(uint8_t direction)
{
    if (direction == 1) // İleri
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);   // IN1 HIGH
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); // IN2 LOW
    }
    else // Geri
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); // IN1 LOW
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);   // IN2 HIGH
    }
}

// Belirtilen frekansta buzzer tonu oluştur
void play_tone(uint32_t frequency)
{
    uint32_t period_us = 1000000 / frequency; // Periyot (mikrosaniye)
    uint32_t half_period_us = period_us / 2; // Yarım periyot

    for (int i = 0; i < 500; i++) // Döngüyü kısa tutun veya bitirene kadar çalıştırın
    {
        HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_SET);
        delay_us(half_period_us);
        HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_RESET);
        delay_us(half_period_us);
    }
}
// Echo pini için pulse süresini okuma fonksiyonu
uint32_t read_pulse_duration(void)
{
  while (HAL_GPIO_ReadPin(GPIOA, ECHO_PIN) == GPIO_PIN_RESET);

  uint32_t start = DWT->CYCCNT;

  while (HAL_GPIO_ReadPin(GPIOA, ECHO_PIN) == GPIO_PIN_SET);

  uint32_t end = DWT->CYCCNT;

  return (end - start) / (HAL_RCC_GetHCLKFreq() / 1000000);
}

uint32_t read_pot_value(void)
{
  HAL_ADC_Start(&hadc1); // ADC'yi başlat
  if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
  {
    uint32_t raw_value = HAL_ADC_GetValue(&hadc1); // ADC değerini oku
    return raw_value; // Ham ADC değerini döndür
  }
  return 0; // Başarısız durumda 0 döndür
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
