/*******************************************************************************
* File:    rfid.c 
* Author:  FarSight Design Team
* Version: V1.00
* Date:    2011.06.21
* Brief:   RFID program
*******************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "LPC11xx.h"
#include "gpio.h"
#include "ssp.h"
#include "oled2864.h"
#include "key.h"
#include "tools.h"
#include "rfid.h"
#include "menu.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const uint8_t RFID_READ_MOD_TYPE_01[2] = {0x02, 0x01};
const uint8_t RFID_READ_TYPE_19[2] = {0x02, 0x19};
const uint8_t RFID_READ_CARD_20[2] = {0x02, 0x20};
const uint8_t RFID_READ_DATA_BLOCK_21[10] =  {0x0a, 0x21, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 
const uint8_t RFID_WRITE_DATA_BLOCK_22[26] = {0x1a, 0x22, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                              0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
										      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f}; 


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : RFID_CheckSum
* Description    : Init RFID control.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t RFID_CheckSum(uint8_t *databuf)
{
 	unsigned char	numb, chksum=0;;

 	numb = databuf[0]-1;

	for(numb=0;numb<databuf[0];numb++)
	{
    	chksum += databuf[numb]; 
	} 
  	return chksum;
}

 static unsigned short SPI0_Send( unsigned char  portNum, unsigned char  buf )
{
 	
  
	if ( portNum == 0 )
	{
	  GPIOSetValue( PORT2, 7, 0 ); 
	  while (( !(LPC_SSP0->SR & SSPSR_TNF)||(LPC_SSP0->SR & SSPSR_BSY)) != 0 );
	  LPC_SSP0->DR = buf;
	  while ( LPC_SSP0->SR & SSPSR_BSY );
	      /* Wait until the Busy bit is cleared */
     while((LPC_SSP0->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE);
   GPIOSetValue( PORT2, 7, 1);
  }
  return	  LPC_SSP0->DR;
}

/*******************************************************************************
* Function Name  : RFID_Operate
* Description    : Output segment to LED.
* Input          : - numb : display number.
* Output         : None
* Return         : None
*******************************************************************************/
 uint8_t RFID_Operate(uint8_t *tbuf, uint8_t *rbuf)
{
  uint8_t	chksum;
  uint32_t 	i, j, rnumb;

  SPI_Init(0, 8, 128);

  chksum = RFID_CheckSum(tbuf);

  SPI0_Send(0, 0xaa);
 
  i = 0;
  rbuf[i] = SPI0_Send(0, 0xbb);
  i ++;

  for(j=0; j<tbuf[0]; j++)
  {
    rbuf[i] = SPI0_Send(0, tbuf[j]);
	i ++;
    delay_ms(1);
  }

  rbuf[i] = SPI0_Send(0, chksum);
  i ++;

   delay_ms(200);

  rbuf[0] = 1;
 
  if(SPI0_Send(0, 0) != 0xaa)
  {
    SPI_Init(0, 8, 2);
    return 0;
  }

  delay_ms(1);
  
  if(SPI0_Send(0, 0) != 0xbb)
  {
    SPI_Init(0, 8, 2);
    return 0;
  }
	
  delay_ms(1);
	 
  switch(tbuf[1])
  {
    case 0x01:
	  rnumb = 8 + 2 + 1;
	  break;

	case 0x20:
	  rnumb = 4 + 2 + 1;
	  break;

	case 0x21:
	  rnumb = 16 + 2 + 1;
	  break;

	case 0x22:
	  rnumb = 2 + 1;
	  break;

	default:
	  rnumb = 4 + 2 + 1;
	  break;
  }

  for(j=0, i=0; j<=rnumb; j++, i++)
  {
    rbuf[i] = SPI0_Send(0, 0);
	delay_ms(1);
  }		
  SPI_Init(0, 8, 2);

  return i;
}

/*******************************************************************************
* Function Name  : ZigBee_Test
* Description    : ZigBee test.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int RFID_Test(uint8_t *rbuf)
{
  uint8_t chksum;
  int i;
  uint8_t buffer[20] = {0};
  uint8_t testbuf[20] = {0};


  GPIOSetDir(PORT2, 8, 0);
  GPIOIntEnable(PORT2, 8);

  if(!GPIOGetValue(PORT2, 8))
  { 
	  delay_ms(200);

      RFID_Operate((uint8_t *)RFID_READ_DATA_BLOCK_21, buffer);

      chksum = RFID_CheckSum(buffer);

      if(chksum == buffer[buffer[0]])
	  {
		
		for (i=0;i<4;i++)
		{
			rbuf[i] = buffer[2+i];
		}
		//rbuf[i] = '\0'; 
        OLED_DisStrLine(2-1 + 6, 0, "Ok     ");
        //printf("\r\nOk ");
		//send_data(rbuf);
		return 1;
      }
  }
  GPIOIntDisable(PORT2,8);

	 return 0;
}

/**** End of File ****/


