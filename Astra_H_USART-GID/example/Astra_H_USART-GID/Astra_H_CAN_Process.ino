//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  PROCESSING CAN-MESSAGES                            //
//                All processing and decoding of CAN messages is here.                 //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

void CAN_message_process(CanMsg *can_msg){
  switch (can_msg->ID)
    {
      case MS_WHEEL_BUTTONS_ID: {
          switch (can_msg->Data[1]) {
            case MS_BTN_STATION: {
                if ((can_msg->Data[0] == BTN_PRESSED) && ((can_msg->Data[2]) == 0x00)) 
 #ifdef DEBUG               
                Serial2.print("\nBTN_STATION Pressed");
 #endif               
                break;
              }
            case MS_BTN_MODE: {
                if ((can_msg->Data[0] == BTN_PRESSED) && ((can_msg->Data[2]) == 0x00)) 
 #ifdef DEBUG                
                Serial2.print("\nBTN_MODE Pressed");
 #endif                
                break;
              }
            case MS_BTN_NEXT: {
                if ((can_msg->Data[0] == BTN_PRESSED) && ((can_msg->Data[2]) == 0x00)) {
 #ifdef DEBUG               
                Serial2.print("\nBTN_NEXT Pressed");
 #endif      
                }
                break;
              }
            case MS_BTN_PREV: {
                if ((can_msg->Data[0] == BTN_PRESSED) && ((can_msg->Data[2]) == 0x00)) {
 #ifdef DEBUG               
                Serial2.print("\nBTN_PREV Pressed");
 #endif                        
                }
                break;
              }
          }
          break;
        }
    case MS_MEDIA_BUTTONS_ID: {
        switch (can_msg->Data[1]) {
          case MS_BTN_OK: {
              break;
            }
         case MS_BTN_RIHGT: {
              break;
            }
         case MS_BTN_LEFT: {
              break;
            }       
        }
        break;
      }
    case MS_MEDIA_ID: {                                                   //If EHU in AUX-Mode
        Pause_Update_DIS = millis();
        if (((can_msg->Data[0]) == 0x10) && ((can_msg->Data[1]) == 0x22) && ((can_msg->Data[2]) != 0x40) && AUX_mode)  {
        //if (((can_msg->Data[0]) == 0x21) && AUX_mode)  {      // test on two-line GID     
          delay(1);
          SendCANmessage(MS_MEDIA_ID, 8, 0x21, 0x3A, 0xC0, 0x00, 0x37, 0x03, 0x10, 0x1A); //Corrupt message
          //SendCANmessage(MS_MEDIA_ID, 8, 0x21, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);   //Corrupt message, test on two-line GID
#ifdef DEBUG
          Serial2.print("\nCorrupt message");
#endif
        }
      
         if ((can_msg->Data[0]) == 0x21){ 
            if ((can_msg->Data[6]) == 0x74){
              Sound=true;}
            else Sound=false;}
      
        if ((can_msg->Data[0]) == 0x24) {
          if (((can_msg->Data[3]) != 0x41) && ((can_msg->Data[5]) != 0x75) && ((can_msg->Data[7]) != 0x78))
          { if (!Sound) {
            AUX_mode = false;
#ifdef DEBUG
            Serial2.print("\nAUX OFF");
#endif
          }}
          else {
            if (AUX_mode == 0) {
              delay(50);
              AUX_mode = 1;
#ifdef DEBUG
              Serial2.print("\nAUX ON");
#endif
            }
          }
        }

        break;
      }
      case MS_ECC_ID: {
          if (can_msg->Data[0] == MS_BATTERY) {
            VOLTAGE = (can_msg->Data[2]);
          }
          if (can_msg->Data[0] == MS_ENGINE_TEMP) {
            T_ENG = (((can_msg->Data[3]) * 256) + (can_msg->Data[4]));
          }
          if (can_msg->Data[0] == MS_RPM_SPEED) {
            RPM = ((can_msg->Data[1]) * 256) + (can_msg->Data[2]);
            SPEED = can_msg->Data[4];
          }
          break;
        }
    }
#ifdef DEBUG
    char scan[40];
    sprintf (scan, "\n%d: %04X # %02X %02X %02X %02X %02X %02X %02X %02X ", millis(),
             can_msg->ID, can_msg->Data[0], can_msg->Data[1], can_msg->Data[2], can_msg->Data[3],
             can_msg->Data[4], can_msg->Data[5], can_msg->Data[6], can_msg->Data[7]);
    Serial2.print(scan);
#endif
  }
