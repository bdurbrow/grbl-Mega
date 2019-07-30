#include <stdlib.h>
#include <math.h>

#include "grbl.h"

#ifdef USE_UI_SUPPORT

#include "DriverUtilities.h"
#include "Keypad_grbl.h"
#include "LiquidCrystal_grbl.h"

#include "SDSupport.h"
#include "UISupport.h"
#include "UIMenus.h"


char UILineBuffer[UILineBufferSize];
uint8_t UILineBufferState = 0;

char editBuffer[10];
uint8_t editBufferState;

static inline float readAxisPosition_MachinePosition(uint8_t axisIndex)
{
  int32_t current_position[N_AXIS]; // Copy current state of the system position variable
  uint8_t oldSREG = SREG;
  cli();
    #ifdef COREXY
      memcpy(current_position,sys_position,sizeof(sys_position));  
    #else
      current_position[axisIndex] = sys_position[axisIndex];
    #endif
  SREG = oldSREG;
  
  return system_convert_axis_steps_to_mpos(current_position, axisIndex);
}

static inline float readAxisPosition_WorkPosition(uint8_t axisIndex)
{
  int32_t current_position[N_AXIS]; // Copy current state of the system position variable
  uint8_t oldSREG = SREG;
  cli();
    #ifdef COREXY
      memcpy(current_position,sys_position,sizeof(sys_position));  
    #else
      current_position[axisIndex] = sys_position[axisIndex];
    #endif
  SREG = oldSREG;
  
  float workCoordanateOffset =  gc_state.coord_system[axisIndex]+gc_state.coord_offset[axisIndex];
  if (axisIndex == TOOL_LENGTH_OFFSET_AXIS)
    workCoordanateOffset += gc_state.tool_length_offset;
  
  return system_convert_axis_steps_to_mpos(current_position, axisIndex) - workCoordanateOffset;
}

// UI Page Subclass Instances

	DialogUIPage dialogUIPage;
	ErrorUIPage errorUIPage;
	StatusUIPage statusUIPage;
	JogUIPage jogUIPage;
	MenuUIPage menuUIPage;
	FormUIPage formUIPage;
	FileDirectoryUIPage fileDirectoryUIPage;
	MDIUIPage mdiUIPage;
	ToolDataUIPage toolDataUIPage;
	RPNCalculatorUIPage rpnCalculatorUIPage;

#define UI_SHIFT_INDICATOR_CHARACTER    ((uint8_t)0)
#define UI_MENU_INDICATOR_CHARACTER     ((uint8_t)1)
#define UI_MENU_PAGE_ICON_CHARACTER     ((uint8_t)2)
#define UI_STOP_INDICATOR_CHARACTER     ((uint8_t)3)
#define UI_RUN_INDICATOR_CHARACTER      ((uint8_t)4)
#define UI_COOLANT_INDICATOR_CHARACTER  ((uint8_t)5)
#define UI_BACKSLASH_CHARACTER          ((uint8_t)6)


#define JOG_KEYVALUE                'j'
#define PROBEPOS_KEYVALUE           'p'
#define PROBENEG_KEYVALUE           'n'
#define DELETE_KEYVALUE             'd'
#define MENU_KEYVALUE               'm'
#define MDI_KEYVALUE                'i'
#define ENTER_KEYVALUE              'e'
#define SHIFTED_ENTER_KEYVALUE      'E'
#define PARTZERO_KEYVALUE           'z'
#define SHIFTED_PARTZERO_KEYVALUE   '!'
#define SHIFT_KEYVALUE              's'


const uint16_t PROGMEM keys[4][4] = {
  {KEYVALUE('7', 'G'), KEYVALUE('8', 'X'), KEYVALUE('9', JOG_KEYVALUE), KEYVALUE(MENU_KEYVALUE, MDI_KEYVALUE) },
  {KEYVALUE('4', 'M'), KEYVALUE('5', 'Y'), KEYVALUE('6', PROBEPOS_KEYVALUE), KEYVALUE(ENTER_KEYVALUE, SHIFTED_ENTER_KEYVALUE) },
  {KEYVALUE('1', 'F'), KEYVALUE('2', 'Z'), KEYVALUE('3', PROBENEG_KEYVALUE), KEYVALUE(PARTZERO_KEYVALUE, SHIFTED_PARTZERO_KEYVALUE) },
  {KEYVALUE('0', 'S'), KEYVALUE('.', 'L'), KEYVALUE('-', DELETE_KEYVALUE), KEYVALUE(SHIFT_KEYVALUE, SHIFT_KEYVALUE) }
};
Keypad keypad = Keypad(makeKeymap(keys));

LiquidCrystal lcd;

AbstractUIPage *activeUIPage;


  void UISetup(void)
  {
    lcd.begin();
    
    byte shiftIndicatorCharacterImage[8] = 
    {
      0b00100,
      0b01110,
      0b11111,
      0b00100,
      0b00100,
      0b00100,
      0b00000,
      0b00000
    };
    lcd.createChar(UI_SHIFT_INDICATOR_CHARACTER, shiftIndicatorCharacterImage);
    
    byte menuIndicatorCharacterImage[8] = 
    {
      0b01000,
      0b00100,
      0b00110,
      0b11111,
      0b00110,
      0b00100,
      0b01000,
      0b00000
    };
    lcd.createChar(UI_MENU_INDICATOR_CHARACTER, menuIndicatorCharacterImage);
        
    byte menuPageIconCharacterImage[8] = 
    {
      0b00000,
      0b11110,
      0b00000,
      0b11110,
      0b00000,
      0b11110,
      0b00000,
      0b00000
    };
    lcd.createChar(UI_MENU_PAGE_ICON_CHARACTER, menuPageIconCharacterImage);
    
    byte stopIndicatorCharacterImage[8] = 
    {
      0b00000,
      0b11111,
      0b11111,
      0b11111,
      0b11111,
      0b11111,
      0b00000,
      0b00000
    };
    lcd.createChar(UI_STOP_INDICATOR_CHARACTER, stopIndicatorCharacterImage);
    
    byte runIndicatorCharacterImage[8] = 
    {
      0b01000,
      0b01100,
      0b01110,
      0b01111,
      0b01110,
      0b01100,
      0b01000,
      0b00000
    };
    lcd.createChar(UI_RUN_INDICATOR_CHARACTER, runIndicatorCharacterImage);
    
    byte coolantIndicatorCharacterImage[8] = 
    {
      0b01010,
      0b01010,
      0b01010,
      0b01010,
      0b10001,
      0b00000,
      0b01000,
      0b00010
    };
    lcd.createChar(UI_COOLANT_INDICATOR_CHARACTER, coolantIndicatorCharacterImage);
    
    byte backslashCharacterImage[8] = 
    {
      0b00000,
      0b10000,
      0b01000,
      0b00100,
      0b00010,
      0b00001,
      0b00000,
      0b00000
    };
    lcd.createChar(UI_BACKSLASH_CHARACTER, backslashCharacterImage);
    
    activeUIPage=&statusUIPage;
    statusUIPage.display();
    
    // Setup the encoder input pins
    setPinMode(UI_ENCODER_A_PINBTN1, INPUT);
    #ifdef UI_ENCODER_A_PINBTN1_ENABLE_PULLUP
      fastDigitalWrite(UI_ENCODER_A_PINBTN1, HIGH);
    #else
      fastDigitalWrite(UI_ENCODER_A_PINBTN1, LOW);
    #endif
    
    setPinMode(UI_ENCODER_A_PINBTN2, INPUT);
    #ifdef UI_ENCODER_A_PINBTN2_ENABLE_PULLUP
      fastDigitalWrite(UI_ENCODER_A_PINBTN2, HIGH);
    #else
      fastDigitalWrite(UI_ENCODER_A_PINBTN2, LOW);
    #endif
    
    setPinMode(UI_ENCODER_A_PIN1, INPUT);
    #ifdef UI_ENCODER_A_PIN1_ENABLE_PULLUP
      fastDigitalWrite(UI_ENCODER_A_PIN1, HIGH);
    #else
      fastDigitalWrite(UI_ENCODER_A_PIN1, LOW);
    #endif
    
    setPinMode(UI_ENCODER_A_PIN2, INPUT);
    #ifdef UI_ENCODER_A_PIN2_ENABLE_PULLUP
      fastDigitalWrite(UI_ENCODER_A_PIN2, HIGH);
    #else
      fastDigitalWrite(UI_ENCODER_A_PIN2, LOW);
    #endif
    
    #ifdef USE_UI_ENCODER_B
      setPinMode(UI_ENCODER_B_PIN1, INPUT);
      #ifdef UI_ENCODER_B_PIN1_ENABLE_PULLUP
        fastDigitalWrite(UI_ENCODER_B_PIN1, HIGH);
      #else
        fastDigitalWrite(UI_ENCODER_B_PIN1, LOW);
      #endif
    
      setPinMode(UI_ENCODER_B_PIN2, INPUT);
      #ifdef UI_ENCODER_B_PIN2_ENABLE_PULLUP
        fastDigitalWrite(UI_ENCODER_B_PIN2, HIGH);
      #else
        fastDigitalWrite(UI_ENCODER_B_PIN2, LOW);
      #endif

      #ifdef USE_UI_ENCODER_B_ISR
        #if(UI_ENCODER_B_MODE == 1)
          EICRB &=~ B01;
          EICRB |= B10;
          EIMSK |= B10000;
        #elif(UI_ENCODER_B_MODE == 2)
          EICRB |= B11;
          EIMSK |= B10000;
        #else
          EICRB &=~ B1010;
          EICRB |= B0101;
          EIMSK |= B110000;
        #endif
      #endif
      
    #endif    
  }

  #define PollUIEncoderMode1(encoderIdentifer)                                                              \
    {                                                                                                       \
      uint8_t PreviousUIEncoderState##encoderIdentifer = ( UIEncoderState##encoderIdentifer << 2 );         \
      UIEncoderState##encoderIdentifer =    fastDigitalRead( UI_ENCODER_##encoderIdentifer##_PIN1 )         \
                                        | ( fastDigitalRead( UI_ENCODER_##encoderIdentifer##_PIN2 ) << 1);  \
      switch(PreviousUIEncoderState##encoderIdentifer | UIEncoderState##encoderIdentifer)                   \
      {                                                                                                     \
        case B1110:                                                                                         \
          activeUIPage-> UIEncoder##encoderIdentifer##Left ();                                              \
        break;                                                                                              \
                                                                                                            \
        case B1101:                                                                                         \
          activeUIPage-> UIEncoder##encoderIdentifer##Right ();                                             \
        break;                                                                                              \
                                                                                                            \
        default:                                                                                            \
        break;                                                                                              \
      }                                                                                                     \
    }                                                                                                       \
    
  #define PollUIEncoderMode2(encoderIdentifer)                                                              \
    {                                                                                                       \
      uint8_t PreviousUIEncoderState##encoderIdentifer = ( UIEncoderState##encoderIdentifer << 2 );         \
      UIEncoderState##encoderIdentifer =    fastDigitalRead( UI_ENCODER_##encoderIdentifer##_PIN1 )         \
                                        | ( fastDigitalRead( UI_ENCODER_##encoderIdentifer##_PIN2 ) << 1);  \
      switch(PreviousUIEncoderState##encoderIdentifer | UIEncoderState##encoderIdentifer)                   \
      {                                                                                                     \
        case B0001:                                                                                         \
          activeUIPage-> UIEncoder##encoderIdentifer##Left ();                                              \
        break;                                                                                              \
                                                                                                            \
        case B0010:                                                                                         \
          activeUIPage-> UIEncoder##encoderIdentifer##Right ();                                             \
        break;                                                                                              \
                                                                                                            \
        default:                                                                                            \
        break;                                                                                              \
      }                                                                                                     \
    }                                                                                                       \
    
  #define PollUIEncoderMode3(encoderIdentifer)                                                              \
    {                                                                                                       \
      uint8_t PreviousUIEncoderState##encoderIdentifer = ( UIEncoderState##encoderIdentifer << 2 );         \
      UIEncoderState##encoderIdentifer =    fastDigitalRead( UI_ENCODER_##encoderIdentifer##_PIN1 )         \
                                        | ( fastDigitalRead( UI_ENCODER_##encoderIdentifer##_PIN2 ) << 1);  \
      switch(PreviousUIEncoderState##encoderIdentifer | UIEncoderState##encoderIdentifer)                   \
      {                                                                                                     \
        case B0001:                                                                                         \
        case B0111:                                                                                         \
        case B1110:                                                                                         \
        case B1000:                                                                                         \
          activeUIPage-> UIEncoder##encoderIdentifer##Left ();                                              \
        break;                                                                                              \
                                                                                                            \
        case B0010:                                                                                         \
        case B1011:                                                                                         \
        case B1101:                                                                                         \
        case B0100:                                                                                         \
          activeUIPage-> UIEncoder##encoderIdentifer##Right ();                                             \
        break;                                                                                              \
                                                                                                            \
        default:                                                                                            \
        break;                                                                                              \
      }                                                                                                     \
    }                                                                                                       \
    
    
  #ifdef USE_UI_ENCODER_B_ISR
    volatile int8_t UIEncoderBAccumulator = 0;
    
    #if((UI_ENCODER_B_MODE == 1) || (UI_ENCODER_B_MODE == 2))
      ISR(INT4_vect)
      {
        if(fastDigitalRead(3))
          UIEncoderBAccumulator++;
        else
          UIEncoderBAccumulator--;
      }
    #else
    
			#define PollUIEncoderMode3_accumulate(encoderIdentifer)                                                   \
				{                                                                                                       \
					uint8_t PreviousUIEncoderState##encoderIdentifer = ( UIEncoderState##encoderIdentifer << 2 );         \
					UIEncoderState##encoderIdentifer =    fastDigitalRead( UI_ENCODER_##encoderIdentifer##_PIN1 )         \
																						| ( fastDigitalRead( UI_ENCODER_##encoderIdentifer##_PIN2 ) << 1);  \
					switch(PreviousUIEncoderState##encoderIdentifer | UIEncoderState##encoderIdentifer)                   \
					{                                                                                                     \
						case B0001:                                                                                         \
						case B0111:                                                                                         \
						case B1110:                                                                                         \
						case B1000:                                                                                         \
							UIEncoderBAccumulator--;                                              														\
						break;                                                                                              \
																																																								\
						case B0010:                                                                                         \
						case B1011:                                                                                         \
						case B1101:                                                                                         \
						case B0100:                                                                                         \
							UIEncoderBAccumulator++;                                              														\
						break;                                                                                              \
																																																								\
						default:                                                                                            \
						break;                                                                                              \
					}                                                                                                     \
				}                                                                                                       \
    
      uint8_t UIEncoderStateB;

      ISR(INT4_vect)
      {
				PollUIEncoderMode3_accumulate(B);
      }
      
      ISR(INT5_vect)
      {
				PollUIEncoderMode3_accumulate(B);
      }
    #endif
  #endif
	
	// Count how many phases are needed for UITask.
	
		#if( defined(UI_FEED_OVERRIDE_POTENTIOMETER) )
			#define UITASK_FEED_OVERRIDE_POTENTIOMETER_PHASE (5)
			#define UITASK_PHASE_CTR1	2
		#else
			#define UITASK_PHASE_CTR1	0
		#endif
	
		#if( defined(UI_RAPID_OVERRIDE_POTENTIOMETER) )
			#define UITASK_RAPID_OVERRIDE_POTENTIOMETER_PHASE (5 + UITASK_PHASE_CTR1)
			#define UITASK_PHASE_CTR2	2
		#else
			#define UITASK_PHASE_CTR2	0
		#endif
			
		#if( defined(UI_SPINDLE_OVERRIDE_POTENTIOMETER) )
			#define UITASK_SPINDLE_OVERRIDE_POTENTIOMETER_PHASE (5 + UITASK_PHASE_CTR1 + UITASK_PHASE_CTR2)
			#define UITASK_PHASE_CTR3	2
		#else
			#define UITASK_PHASE_CTR3	0
		#endif
			
		#define UITASK_PHASE_COUNT	(5 + UITASK_PHASE_CTR1 + UITASK_PHASE_CTR2 + UITASK_PHASE_CTR3)
	
  void UITask(void)
  {
    // Encoder B is polled at the full rate possible.
    #if ( defined(USE_UI_ENCODER_B) && !defined(USE_UI_ENCODER_B_ISR) )
      static uint8_t UIEncoderStateB;
      #if(UI_ENCODER_B_MODE == 1)
        PollUIEncoderMode1(B);
      #elif(UI_ENCODER_B_MODE == 2)
        PollUIEncoderMode2(B);
      #else
        PollUIEncoderMode3(B);
      #endif
    #endif
				
    // Rate limits UI polling/updating to the clock_ticks frequency.
    static uint32_t previousTicks = 0;
    uint32_t currentTicks = clock_ticks();
    if( previousTicks == currentTicks ) return;
    previousTicks = currentTicks;
		
    {
      // Poll encoder A
      static uint8_t UIEncoderStateA;
      #if(UI_ENCODER_A_MODE == 1)
        PollUIEncoderMode1(A);
      #elif(UI_ENCODER_A_MODE == 2)
        PollUIEncoderMode2(A);
      #else
        PollUIEncoderMode3(A);
      #endif
    }
		
	  #ifdef USE_UI_ENCODER_B_ISR
      {
        uint8_t oldSREG = SREG;
        cli();
          uint8_t accumulatorValue = UIEncoderBAccumulator;
          UIEncoderBAccumulator = 0;
        SREG = oldSREG;
  
        if(accumulatorValue)
        {
          activeUIPage->UIEncoderBCount(accumulatorValue);
          return;
        }
      }
    #endif
		
		static uint8_t uiTaskPhase = 0;
		switch(uiTaskPhase)
		{
			case 0:
				{
					activeUIPage->update();
				}
				break;
			
			case 1:
				{
					// Poll the control inputs.
					uint8_t pin = system_control_get_state();
					if (pin) {
						if (bit_istrue(pin,CONTROL_PIN_INDEX_RESET)) {
							mc_reset();
						}
						if (bit_istrue(pin,CONTROL_PIN_INDEX_CYCLE_START)) {
							system_set_exec_state_flag(EXEC_CYCLE_START);
						}
						#ifndef ENABLE_SAFETY_DOOR_INPUT_PIN
							if (bit_istrue(pin,CONTROL_PIN_INDEX_FEED_HOLD)) {
								system_set_exec_state_flag(EXEC_FEED_HOLD);
						  }
						#else
							if (bit_istrue(pin,CONTROL_PIN_INDEX_SAFETY_DOOR)) {
								system_set_exec_state_flag(EXEC_SAFETY_DOOR);
							}
						#endif
					}
				}
				break;
				
			case 2:
				{
					// Poll the keypad.
					static bool shiftState = false;
					uint8_t key = shiftState?SHIFTED(keypad.getKey()):UNSHIFTED(keypad.getKey());
					
					if(key)
					{
						switch(key)
						{
							case JOG_KEYVALUE:
								{
									switch (sys.state)
									{
										case STATE_IDLE:
										case STATE_CHECK_MODE:
										case STATE_JOG:
											if(UILineBufferIsAvailable())
												goToUIPage(&jogUIPage);
											break;
							
										case STATE_CYCLE:
										case STATE_HOMING:
											UIShowError(PSTR("  Program Running.  "));
											break;

										case STATE_HOLD:
											UIShowError(PSTR("  Feed Hold Active. "));
											break;

										case STATE_ALARM:
											UIShowError(PSTR("    Alarm Active.   "));
											break;

										case STATE_SAFETY_DOOR:
											UIShowError(PSTR("     Door Open.     "));
											break;
								
										case STATE_SLEEP:
											UIShowError(PSTR("     Sleep Mode.    "));
											break;
									}
								}
								break;
			
							case PROBEPOS_KEYVALUE:
								activeUIPage->probePosKeyPressed();
								break;
		
							case PROBENEG_KEYVALUE:
								activeUIPage->probeNegKeyPressed();
								break;
			
							case DELETE_KEYVALUE:
								activeUIPage->deleteKeyPressed();
								break;
		
							case MENU_KEYVALUE:
								menuUIPage.showMenu((UIMenuEntry *)mainMenu);
								break;
		
							case MDI_KEYVALUE:
								if(UILineBufferIsAvailable())
									goToUIPage(&mdiUIPage);
								break;
		
							case ENTER_KEYVALUE:
								activeUIPage->enterKeyPressed();
								break;
		
							case SHIFTED_ENTER_KEYVALUE:
								activeUIPage->shiftedEnterKeyPressed();
								break;
		
							case PARTZERO_KEYVALUE:
								activeUIPage->partZeroKeyPressed();
								break;
					
							case SHIFTED_PARTZERO_KEYVALUE:
								activeUIPage->shiftedPartZeroKeyPressed();
								break;
		
							case SHIFT_KEYVALUE:
								shiftState = !shiftState;
								lcd.setCursor(19,0);
								lcd.write(shiftState?UI_SHIFT_INDICATOR_CHARACTER:' ');
								return;
			
							default:
								activeUIPage->keyPressed(key);
								break;
						}
			
						if(shiftState)
						{
							shiftState = false;
							lcd.setCursor(19,0);
							lcd.write(' ');
						};
					}
				}
				break;
				
			case 3:
				{
					// Poll UI Encoder Button 1
					static bool previousUIEncoderButton1State = false;
					#ifdef UI_ENCODER_A_PINBTN1_ACTIVE_LOW
						bool currentUIEncoderButton1State = !fastDigitalRead(UI_ENCODER_A_PINBTN1);
					#else
						bool currentUIEncoderButton1State = fastDigitalRead(UI_ENCODER_A_PINBTN1);
					#endif
					if( (!previousUIEncoderButton1State) && currentUIEncoderButton1State )
						activeUIPage->UIEncoderButton1Pressed();
					previousUIEncoderButton1State = currentUIEncoderButton1State;
				}
				break;
				
			case 4:
				{
					// Poll UI Encoder Button 2
					static bool previousUIEncoderButton2State = false;
					#ifdef UI_ENCODER_A_PINBTN2_ACTIVE_LOW
						bool currentUIEncoderButton2State = !fastDigitalRead(UI_ENCODER_A_PINBTN2);
					#else
						bool currentUIEncoderButton2State = fastDigitalRead(UI_ENCODER_A_PINBTN2);
					#endif
					if( (!previousUIEncoderButton2State) && currentUIEncoderButton2State )
						activeUIPage->UIEncoderButton2Pressed();
					previousUIEncoderButton2State = currentUIEncoderButton2State;
				}
				break;
							
			#if( defined(UI_FEED_OVERRIDE_POTENTIOMETER) )
				case UITASK_FEED_OVERRIDE_POTENTIOMETER_PHASE:
					{
						analogStartConversion(UI_FEED_OVERRIDE_POTENTIOMETER, ADC_INTERNAL2V56, 8);
					}
					break;
				case UITASK_FEED_OVERRIDE_POTENTIOMETER_PHASE + 1:
					{
						static int8_t previousFeedOverridePotValue = 0;
						int8_t currentFeedOverridePotValue = (((int8_t)(analogRead_8bits() >> 1)) - 64);
						if((currentFeedOverridePotValue > -2) && (currentFeedOverridePotValue < 2)) currentFeedOverridePotValue = 0; // "Snap" to zero.
						if(abs(previousFeedOverridePotValue - currentFeedOverridePotValue) > 2)	// Adds some hysteresis to the reading to "de-noise" it somewhat.
						{
							previousFeedOverridePotValue = currentFeedOverridePotValue;
							uint8_t newOverride = 100 + currentFeedOverridePotValue;
							newOverride = min(newOverride, MAX_FEED_RATE_OVERRIDE);
							newOverride = max(newOverride, MIN_FEED_RATE_OVERRIDE);
							if(newOverride != sys.f_override)
							{
								sys.f_override = newOverride;
								plan_update_velocity_profile_parameters();
								plan_cycle_reinitialize();
								sys.report_ovr_counter = 0; // Set to report change immediately
							}
						}
					}
					break;
			#endif
			
			#if( defined(UI_RAPID_OVERRIDE_POTENTIOMETER) )
				case UITASK_RAPID_OVERRIDE_POTENTIOMETER_PHASE:
					{
						analogStartConversion(UI_RAPID_OVERRIDE_POTENTIOMETER, ADC_INTERNAL2V56, 8);
					}
					break;
				case UITASK_RAPID_OVERRIDE_POTENTIOMETER_PHASE + 1:
					{
						static int8_t previousRapidOverridePotValue = 0;
						int8_t currentRapidOverridePotValue = ((int8_t)(analogRead_8bits() >> 2));
						if(currentRapidOverridePotValue < 3) currentRapidOverridePotValue = 0; // "Snap" to zero.
						if(abs(previousRapidOverridePotValue - currentRapidOverridePotValue) > 2)	// Adds some hysteresis to the reading to "de-noise" it somewhat.
						{
							previousRapidOverridePotValue = currentRapidOverridePotValue;
							uint8_t newOverride = 100 - currentRapidOverridePotValue;
							if(newOverride != sys.r_override)
							{
								sys.r_override = newOverride;
								plan_update_velocity_profile_parameters();
								plan_cycle_reinitialize();
								sys.report_ovr_counter = 0; // Set to report change immediately
							}
						}
					}
					break;
			#endif
			
			#if( defined(UI_SPINDLE_OVERRIDE_POTENTIOMETER) )
				case UITASK_SPINDLE_OVERRIDE_POTENTIOMETER_PHASE:
					{
						analogStartConversion(UI_SPINDLE_OVERRIDE_POTENTIOMETER, ADC_INTERNAL2V56, 8);
					}
					break;
				case UITASK_SPINDLE_OVERRIDE_POTENTIOMETER_PHASE + 1:
					{
						static int8_t previousSpindleOverridePotValue = 0;
						int8_t currentSpindleOverridePotValue = (((int8_t)(analogRead_8bits() >> 1)) - 64);
						if(abs(previousSpindleOverridePotValue - currentSpindleOverridePotValue) > 2)	// Adds some hysteresis to the reading to "de-noise" it somewhat.
						{
						  if((currentSpindleOverridePotValue > -2) && (currentSpindleOverridePotValue < 2)) currentSpindleOverridePotValue = 0; // "Snap" to zero.
							previousSpindleOverridePotValue = currentSpindleOverridePotValue;
							uint8_t newOverride = 100 + currentSpindleOverridePotValue;
							newOverride = min(newOverride, MAX_SPINDLE_SPEED_OVERRIDE);
							newOverride = max(newOverride, MIN_SPINDLE_SPEED_OVERRIDE);
							if(newOverride != sys.spindle_speed_ovr)
							{
								sys.spindle_speed_ovr = newOverride;
								if (sys.state == STATE_IDLE) { spindle_set_state(gc_state.modal.spindle, gc_state.spindle_speed); }
								else { bit_true(sys.step_control, STEP_CONTROL_UPDATE_SPINDLE_PWM); }
								sys.report_ovr_counter = 0; // Set to report change immediately
							}
						}					
					}
					break;
			#endif
		}
		
		if(++uiTaskPhase >= UITASK_PHASE_COUNT) uiTaskPhase = 0;
  }

  bool UIStatusNeedsUpdateFlag = false;

  void UIStatusNeedsUpdate(void)
  {
    UIStatusNeedsUpdateFlag = true;
  }

  void UIBufferExecuted(void)
  {
    activeUIPage->lineBufferExecuted();
  };

// DRO Handling
  char DROTextBuffer[10];
  static uint8_t DROUpdatePhase = 0;

	static inline void drawDROText(uint8_t x, uint8_t y, uint8_t characters)
  {
    lcd.setCursor(x,y);
    lcd.write(DROTextBuffer, characters);
  }

  static inline void renderXDROText(void)
  {
    float value = readAxisPosition_WorkPosition(X_AXIS);
    DROTextBuffer[0] = 'X';
    sPrintFloatRightJustified(&DROTextBuffer[1], 9, value, 4);
  }

  static inline void renderYDROText(void)
  {
    float value = readAxisPosition_WorkPosition(Y_AXIS);
    DROTextBuffer[0] = 'Y';
    sPrintFloatRightJustified(&DROTextBuffer[1], 9, value, 4);
  }

  static inline void renderZDROText(void)
  {
    float value = readAxisPosition_WorkPosition(Z_AXIS);
    DROTextBuffer[0] = 'Z';
    sPrintFloatRightJustified(&DROTextBuffer[1], 9, value, 4);
  }

  static inline void renderLineCountDROText(void)
  {
    DROTextBuffer[0] = ' ';
		DROTextBuffer[1] = 'L';
		DROTextBuffer[2] = 'n';
    sPrintFloatRightJustified(&DROTextBuffer[3], 7, SD_line_count, 0);
  }

  #if(N_AXIS > 3)

    static inline void renderADROText(void)
    {
      float value = readAxisPosition_WorkPosition(A_AXIS);
      DROTextBuffer[0] = ' ';
      DROTextBuffer[1] = 'A';
      sPrintFloatRightJustified(&DROTextBuffer[2], 8, value, 3);
    }

  #endif

  #if(N_AXIS > 4)

    static inline void renderBDROText(void)
    {
      float value = readAxisPosition_WorkPosition(B_AXIS);
      DROTextBuffer[0] = ' ';
      DROTextBuffer[1] = 'B';
      sPrintFloatRightJustified(&DROTextBuffer[2], 8, value, 3);
    }

  #endif

  static inline void displayPositionDRO(void)
  {
    renderXDROText(); drawDROText(0,1,10);
    renderYDROText(); drawDROText(0,2,10);
    renderZDROText(); drawDROText(0,3,10);
    
    #if(N_AXIS > 3)
      renderADROText(); drawDROText(10,1,10);
    #else
      lcd.setCursor(10,1);  lcd.writeMultiple(' ', 10);
    #endif
    #if(N_AXIS > 4)
      renderBDROText(); drawDROText(10,2,10);
    #else
      lcd.setCursor(10,2);  lcd.writeMultiple(' ', 10);
    #endif
    
    renderLineCountDROText(); drawDROText(10,3,10);
  }
  
  #if(N_AXIS > 4)
    #define POSITION_DRO_UPDATE_PHASE_COUNT  (12)
  #elif(N_AXIS > 3)
    #define POSITION_DRO_UPDATE_PHASE_COUNT  (10)
  #else
    #define POSITION_DRO_UPDATE_PHASE_COUNT  (8)
  #endif

  static inline void updatePositionDRO(void)
  {
    switch (DROUpdatePhase)
    {
      case 0: renderXDROText(); break;
      case 1: drawDROText(0,1,10); break;
      case 2: renderYDROText(); break;
      case 3: drawDROText(0,2,10); break;
      case 4: renderZDROText(); break;
      case 5: drawDROText(0,3,10); break;
      case 6: renderLineCountDROText(); break;
      case 7: drawDROText(10,3,10); break;
      
      #if(N_AXIS > 3)
        case 8: renderADROText(); break;
        case 9: drawDROText(10,1,10); break;
      #endif
      #if(N_AXIS > 4)
        case 10: renderBDROText(); break;
        case 11: drawDROText(10,2,10); break;
      #endif
    };
    DROUpdatePhase++;
    if(DROUpdatePhase >= POSITION_DRO_UPDATE_PHASE_COUNT)
      DROUpdatePhase = 0;
  }
  
  static inline void renderGCGroup_FeedRate_DROText(void)
  {
		DROTextBuffer[0] = 'F';
    sPrintFloatRightJustified(&DROTextBuffer[1], 8, gc_state.feed_rate, 0);
		DROTextBuffer[9] = ' ';
  }
  
  static inline void renderGCGroup_SpindleRate_DROText(void)
  {
		DROTextBuffer[0] = 'S';
    sPrintFloatRightJustified(&DROTextBuffer[1], 9, gc_state.spindle_speed, 0);
  }
  
	static inline void renderGCGroup_Motion_DROText(void)
	{
		DROTextBuffer[0] = 'G';
	  if (gc_state.modal.motion >= MOTION_MODE_PROBE_TOWARD)
	  {
			DROTextBuffer[1] = '3';
			DROTextBuffer[2] = '8';
			DROTextBuffer[3] = '.';
			DROTextBuffer[4] = '0' + (gc_state.modal.motion - (MOTION_MODE_PROBE_TOWARD-2));
  	}
  	else
  	{
    	DROTextBuffer[1] = '0' + gc_state.modal.motion;
			DROTextBuffer[2] = ' ';
			DROTextBuffer[3] = ' ';
			DROTextBuffer[4] = ' ';
		}
		
		DROTextBuffer[5] = ' ';
	}

	static inline void renderGCGroup_WCS_DROText(void)
	{
		DROTextBuffer[0] = 'G';
		DROTextBuffer[1] = '5';
		DROTextBuffer[2] = '4' + (gc_state.modal.coord_select);
		DROTextBuffer[3] = ' ';
	}
	
	static inline void renderGCGroup_PlaneSelect_DROText(void)
	{
		DROTextBuffer[0] = 'G';
		DROTextBuffer[1] = '1';
		DROTextBuffer[2] = '7' + (gc_state.modal.plane_select);
		DROTextBuffer[3] = ' ';
	}
	
	static inline void renderGCGroup_SpindleMode_DROText(void)
	{
		DROTextBuffer[0] = 'M';
    switch (gc_state.modal.spindle)
    {
      case SPINDLE_ENABLE_CW : DROTextBuffer[1] = '3'; break;
      case SPINDLE_ENABLE_CCW : DROTextBuffer[1] = '4'; break;
      case SPINDLE_DISABLE : DROTextBuffer[1] = '5'; break;
    }
		DROTextBuffer[2] = ' ';
	}
	
	static inline void renderGCGroup_ActiveTool_DROText(void)
	{
		DROTextBuffer[0] = 'T';
		if(gc_state.tool < 10)
		{
		  DROTextBuffer[1] = '0';
		  DROTextBuffer[2] = '0' + gc_state.tool;
		}
		else if(gc_state.tool < 20)
		{
		  DROTextBuffer[1] = '1';
		  DROTextBuffer[2] = '0' + (gc_state.tool - 10);
		}
		else if(gc_state.tool < 30)
		{
		  DROTextBuffer[1] = '2';
		  DROTextBuffer[2] = '0' + (gc_state.tool - 20);
		}
		else
		{
		  DROTextBuffer[1] = '?';
		  DROTextBuffer[2] = '?';
		}
	}
	
	static inline void renderGCGroup_Units_DROText(void)
	{
		DROTextBuffer[0] = 'G';
		DROTextBuffer[1] = '2';
		DROTextBuffer[2] = '1' - (gc_state.modal.units);
		DROTextBuffer[3] = ' ';
		DROTextBuffer[4] = ' ';
		DROTextBuffer[5] = ' ';
	}
	
	static inline void renderGCGroup_AbsIncr_DROText(void)
	{
		DROTextBuffer[0] = 'G';
		DROTextBuffer[1] = '9';
		DROTextBuffer[2] = '0' + (gc_state.modal.distance);
		DROTextBuffer[3] = ' ';
	}
	
	static inline void renderGCGroup_FeedMode_DROText(void)
	{
		DROTextBuffer[0] = 'G';
		DROTextBuffer[1] = '9';
		DROTextBuffer[2] = '4' - (gc_state.modal.feed_rate);
		DROTextBuffer[3] = ' ';
	}
		
	static inline void renderGCGroup_CoolantMode_DROText(void)
	{
	  switch(gc_state.modal.coolant)
	  {
	    case 0:
	      DROTextBuffer[0] = 'M';
		    DROTextBuffer[1] = '9';
		    DROTextBuffer[2] = ' ';
        DROTextBuffer[3] = ' ';
        DROTextBuffer[4] = ' ';
        DROTextBuffer[5] = ' ';
	      break;
	      
	    case PL_COND_FLAG_COOLANT_FLOOD:
	      DROTextBuffer[0] = 'M';
		    DROTextBuffer[1] = '8';
		    DROTextBuffer[2] = ' ';
        DROTextBuffer[3] = ' ';
        DROTextBuffer[4] = ' ';
        DROTextBuffer[5] = ' ';
	      break;
	      
	    case PL_COND_FLAG_COOLANT_MIST:
	      DROTextBuffer[0] = 'M';
		    DROTextBuffer[1] = '7';
		    DROTextBuffer[2] = ' ';
        DROTextBuffer[3] = ' ';
        DROTextBuffer[4] = ' ';
        DROTextBuffer[5] = ' ';
	      break;
	      
	    case PL_COND_FLAG_COOLANT_FLOOD | PL_COND_FLAG_COOLANT_MIST:
	      DROTextBuffer[0] = 'M';
		    DROTextBuffer[1] = '7';
		    DROTextBuffer[2] = ' ';
        DROTextBuffer[3] = 'M';
        DROTextBuffer[4] = '8';
        DROTextBuffer[5] = ' ';
	      break;	    
	  }
	}
		
	  //  01234567890123456789
    //0 #No File.          A<
    //1 F  100 S 10000      <
    //2 G38.2 G54 G17 M3 T10<
    //3 G21   G90 G94 M7 M8 < 

  static inline void displayGCodeDRO(void)
  {
    // First Line
      renderGCGroup_FeedRate_DROText(); drawDROText(0,1,10);
      renderGCGroup_SpindleRate_DROText(); drawDROText(10,1,10);
      lcd.writeMultiple(' ', 7);
          
    // Second Line
      renderGCGroup_Motion_DROText(); drawDROText(0,2,6);
      renderGCGroup_WCS_DROText(); drawDROText(6,2,4);
      renderGCGroup_PlaneSelect_DROText(); drawDROText(10,2,4);
      renderGCGroup_SpindleMode_DROText(); drawDROText(14,2,3);
      renderGCGroup_ActiveTool_DROText(); drawDROText(17,2,3);
		
		// Third Line
      renderGCGroup_Units_DROText(); drawDROText(0,3,6);
      renderGCGroup_AbsIncr_DROText(); drawDROText(6,3,4);
      renderGCGroup_FeedMode_DROText(); drawDROText(10,3,4);
      renderGCGroup_CoolantMode_DROText(); drawDROText(14,3,6);
  }
	
  static inline void updateGCStateDRO(void)
  {
    switch (DROUpdatePhase)
    {
      // First Line
        case 0:   renderGCGroup_FeedRate_DROText(); break;
        case 1:   drawDROText(0,1,10); break;
        case 2:   renderGCGroup_SpindleRate_DROText(); break;
        case 3:   drawDROText(10,1,10); break;
        
      // Second Line
        case 4:   renderGCGroup_Motion_DROText(); break;
        case 5:   drawDROText(0,2,6); break;
        case 6:   renderGCGroup_WCS_DROText(); break;
        case 7:   drawDROText(6,2,4); break;
        case 8:   renderGCGroup_PlaneSelect_DROText(); break;
        case 9:   drawDROText(10,2,4); break;
        case 10:  renderGCGroup_SpindleMode_DROText(); break;
        case 11:  drawDROText(14,2,3); break;
        case 12:  renderGCGroup_ActiveTool_DROText(); break;
        case 13:  drawDROText(17,2,3); break;
        
		// Third Line
        case 14:  renderGCGroup_Units_DROText(); break;
        case 15:  drawDROText(0,3,6); break;
        case 16:  renderGCGroup_AbsIncr_DROText(); break;
        case 17:  drawDROText(6,3,4); break;
        case 18:  renderGCGroup_FeedMode_DROText(); break;
        case 19:  drawDROText(10,3,4); break;
        case 20:  renderGCGroup_CoolantMode_DROText(); break;
        case 21:  drawDROText(14,3,6); break;
    }
    DROUpdatePhase++;
    if(DROUpdatePhase >= 22)
      DROUpdatePhase = 0;
  }
  
  static inline void renderRapidOverrideDROText(void)
  {
    if(sys.r_override < 100) itoa(sys.r_override, DROTextBuffer, 10);
  }
  
  static inline void drawRapidOverrideDROText(void)
  {
    lcd.setCursor(0,1);
    lcd.write_p(PSTR("    Rapid: "));
    if(sys.r_override < 100)
    {
      lcd.write(' ');
      lcd.write(DROTextBuffer, 2);
    }
    else
    {
      lcd.write_p(PSTR("100"));
    }
    lcd.write('%');

    lcd.writeMultiple(' ', 5); 
  }
  
  static inline void renderFeedOverrideDROText(void)
  {
    itoa(sys.f_override, DROTextBuffer, 10);
  }
  
  static inline void drawFeedOverrideDROText(void)
  {
    lcd.setCursor(0,2);
    lcd.write_p(PSTR("     Feed: "));
    if(sys.f_override < 100)
    {
      lcd.write(' ');
      lcd.write(DROTextBuffer, 2);
    }
    else
    {
      lcd.write(DROTextBuffer, 3);
    }
    lcd.write('%');

    lcd.writeMultiple(' ', 5); 
  }
  
  static inline void renderSpindleOverrideDROText(void)
  {
    itoa(sys.spindle_speed_ovr, DROTextBuffer, 10);
  }
  
  static inline void drawSpindleOverrideDROText(void)
  {
    lcd.setCursor(0,3);
    lcd.write_p(PSTR("  Spindle: "));
    if(sys.spindle_speed_ovr < 100)
    {
      lcd.write(' ');
      lcd.write(DROTextBuffer, 2);
    }
    else
    {
      lcd.write(DROTextBuffer, 3);
    }
    lcd.write('%');
    
    lcd.writeMultiple(' ', 5); 
  }
  
  static inline void displayOverrideDRO(void)
  {
    renderRapidOverrideDROText(); drawRapidOverrideDROText();
    renderFeedOverrideDROText();  drawFeedOverrideDROText();
    renderSpindleOverrideDROText(); drawSpindleOverrideDROText();
  }
  
  static inline void updateOverrideDRO(void)
  {
    switch (DROUpdatePhase)
    {
      case 0: renderRapidOverrideDROText(); break;
      case 1: drawRapidOverrideDROText(); break;
      case 2: renderFeedOverrideDROText(); break;
      case 3: drawFeedOverrideDROText(); break;
      case 4: renderSpindleOverrideDROText(); break;
      case 5: drawSpindleOverrideDROText(); break;
    }
    
    DROUpdatePhase++;
    if(DROUpdatePhase >= 6)
      DROUpdatePhase = 0;
  }
  
// AbstractUIPage
  void AbstractUIPage::activate(void) {}
  void AbstractUIPage::deactivate(void) {}

  void AbstractUIPage::lineBufferExecuted() {}

  void AbstractUIPage::display(void) {}
  void AbstractUIPage::update(void) {}

  void AbstractUIPage::UIEncoderALeft(void) {}
  void AbstractUIPage::UIEncoderARight(void) {}
  void AbstractUIPage::UIEncoderButton1Pressed(void) {}
  void AbstractUIPage::UIEncoderButton2Pressed(void) {}

  #ifdef USE_UI_ENCODER_B
    #ifndef USE_UI_ENCODER_B_ISR
      void AbstractUIPage::UIEncoderBLeft(void) {}
      void AbstractUIPage::UIEncoderBRight(void) {}
    #else
      void AbstractUIPage::UIEncoderBCount(int8_t) {}
    #endif
  #endif

  void AbstractUIPage::keyPressed(uint8_t) {}
  void AbstractUIPage::enterKeyPressed(void) {}
  void AbstractUIPage::shiftedEnterKeyPressed(void) {}
  void AbstractUIPage::deleteKeyPressed(void) {}
  void AbstractUIPage::probePosKeyPressed(void) {}
  void AbstractUIPage::probeNegKeyPressed(void) {}
  void AbstractUIPage::partZeroKeyPressed(void) {}
  void AbstractUIPage::shiftedPartZeroKeyPressed(void) {}


// DialogUIPage
  void UIShowDialogPage(const char *message, UIDialogResponseFunction okFunction, UIDialogResponseFunction cancelFunction) // String must be located in program memory.
  {
    dialogUIPage.dialogMessage = message;
    dialogUIPage.okFunction = okFunction;
    dialogUIPage.cancelFunction = cancelFunction;
    goToUIPage(&dialogUIPage);
  }

  void DialogUIPage::display(void)
  {
    lcd.setCursor(0,0);
    lcd.writeMultiple(' ', 20);
    lcd.setCursor(0,1);
    lcd.write_p(dialogMessage);
    lcd.setCursor(0,2);
    lcd.writeMultiple(' ', 20);
    lcd.setCursor(0,3);
    lcd.writeMultiple(' ', 20);
  }
  
  void DialogUIPage::enterKeyPressed(void)
  {
    okFunction();
  }
  
  void DialogUIPage::UIEncoderButton2Pressed(void)
  {
    cancelFunction();
  }

// ErrorUIPage
  void UIShowError(const char *errorMessage) // String must be located in program memory.
  {
    errorUIPage.errorMessage = errorMessage;
    goToUIPage(&errorUIPage);
  }

  void ErrorUIPage::display(void)
  {
    lcd.setCursor(0,0);
    lcd.writeMultiple(' ', 20);
    lcd.setCursor(0,1);
    lcd.writeMultiple(' ', 7);
    lcd.write_p(PSTR("Error:"));
    lcd.writeMultiple(' ', 7);
    lcd.setCursor(0,2);
    lcd.write_p(errorMessage);
    lcd.setCursor(0,3);
    lcd.writeMultiple(' ', 20);
  }
  
  void ErrorUIPage::UIEncoderButton1Pressed(void)
  {
    goToUIPage(&statusUIPage);
  }
  
  void ErrorUIPage::UIEncoderButton2Pressed(void)
  {
    goToUIPage(&statusUIPage);
  }
  
  void ErrorUIPage::enterKeyPressed(void)
  {
    goToUIPage(&statusUIPage);
  }

// StatusUIPage
  void StatusUIPage::activate(void)
	{
		subPage = 0;
		runIndicatorCounter=0;
		runIndicatorPhase=0;
		DROUpdatePhase = 0;
	}
	
  void StatusUIPage::display(void)
  {
    drawStatusLine();
  	switch(subPage)
  	{
  		case 0:
				displayPositionDRO();
  			break;
  		case 1:
				displayGCodeDRO();
  			break;
  		case 2:
				displayOverrideDRO();
  			break;
  	}
  }
  
  void StatusUIPage::update(void) 
  {
    static uint8_t previousState=0;
        
    if(!runIndicatorCounter--)
    {
      runIndicatorCounter = 150;
      if((sys.state & (STATE_HOMING | STATE_CYCLE | STATE_JOG)) && !(sys.state & (STATE_ALARM | STATE_CHECK_MODE | STATE_SAFETY_DOOR | STATE_SLEEP)))
      {
        lcd.setCursor(18,0);
        switch(runIndicatorPhase++)
        {
          case 0: lcd.write('-'); break;
          case 1: lcd.write(UI_BACKSLASH_CHARACTER); break;
          case 2: lcd.write('|'); break;
          case 3: lcd.write('/'); runIndicatorPhase=0; break;
        }
      }
      else
      {
        lcd.setCursor(18,0);
        lcd.write(' ');
      }
      return;
    }
    
    if(UIStatusNeedsUpdateFlag || (previousState != sys.state))
    {
      previousState = sys.state;
      UIStatusNeedsUpdateFlag = false;
      drawStatusLine();
      return;
    }
    
  	switch(subPage)
  	{
  		case 0:
    		updatePositionDRO();
  			break;
  		case 1:
  			updateGCStateDRO();
  			break;
  		case 2:
  		  updateOverrideDRO();
  		  break;
  	}
  }

  void StatusUIPage::drawStatusLine(void)
  {
    lcd.setCursor(0,0);
    
    if(SD_state == SD_state_Open)
    {
      lcd.write(UI_RUN_INDICATOR_CHARACTER);
      if(grbltempIsOpen)
      {
      	lcd.write_p(grbltempDisplayName);
      }
      else
			{      
				char *cursor = gcode_file.name();
				uint8_t spacesCount = 8;
				while( *cursor && (*cursor != '.') )
				{
					lcd.write(*cursor);
					cursor++;
					spacesCount--;
				}
				while(spacesCount)
				{
					lcd.write(' ');
					spacesCount--;
				}
			}
		}
    else
    {
      lcd.write(UI_STOP_INDICATOR_CHARACTER);
      lcd.write_p(PSTR("No File."));
    }
    
    if(sys.state & STATE_ALARM)               lcd.write_p(PSTR("  Alarm!"));
    else if(sys.state & STATE_CHECK_MODE)     lcd.write_p(PSTR(" Dry Run"));
    else                                      lcd.writeMultiple(' ', 8);
    
    if (gc_state.modal.coolant)
      lcd.write(UI_COOLANT_INDICATOR_CHARACTER);
    else
      lcd.write(' ');
  }
  
    void StatusUIPage::UIEncoderALeft(void)
    {
      if(subPage > 0) subPage--;
			DROUpdatePhase = 0;
			display();
    }
    
    void StatusUIPage::UIEncoderARight(void)
    {
			if(subPage < 2) subPage++;
			DROUpdatePhase = 0;
			display();
    }
  
// JogUIPage

  #define JogUIPage_positiveDirection_Flag            (0x01)
  #define JogUIPage_negativeDirection_Flag            (0x02)
  #define JogUIPage_positiveDirectionLockout_Flag     (0x04)
  #define JogUIPage_negativeDirectionLockout_Flag     (0x08)
  #define JogUIPage_didProbePositiveDirection_Flag    (0x10)
  #define JogUIPage_didProbeNegativeDirection_Flag    (0x20)

  inline void JogUIPage::updateActiveIncrementValue(void)
  {
    switch(activeIncrement)
    {
      case 0: activeIncrementValue = 0.0001; break;
      case 1: activeIncrementValue = 0.0010; break;
      case 2: activeIncrementValue = 0.0100; break;
      case 3: activeIncrementValue = 0.1000; break;
      case 4: activeIncrementValue = 1.0000; break;
      case 5: activeIncrementValue = 10.000; break;
    }
  };

  void JogUIPage::activate(void)
  {
		if(sys.state & STATE_JOG)
			system_set_exec_state_flag(EXEC_MOTION_CANCEL); 
  	
		DROUpdatePhase = 0;
    activeAxis = 0;
    motionFlags = 0;
    if(gc_state.modal.units)
    {
      // inch
      activeIncrement = UI_JOGGING_INITIAL_INCH_RANGE;
    }
    else
    {
      // mm
      activeIncrement = UI_JOGGING_INITIAL_METRIC_RANGE;
    }
    
    updateActiveIncrementValue();
    
    performingPowerFeed = false;
    editingJogFeed = false;
  }
  
  void JogUIPage::deactivate(void)
  {
    if(sys.state & STATE_JOG)
      system_set_exec_state_flag(EXEC_MOTION_CANCEL); 
  }

  void JogUIPage::display(void)
  {
    drawStatusLine();
    
    lcd.setCursor(10,1);
    lcd.writeMultiple(' ', 10);
    
    displayPositionDRO();
  }
  
  void JogUIPage::update(void) 
  {
    if(performingPowerFeed && !(sys.state & STATE_JOG) ) performingPowerFeed = false;
    updatePositionDRO();
  }
  
  inline void JogUIPage::jog(int8_t count)
  {
    //print_uint8_base2_ndigit(motionFlags, 8);
    //printInteger(count);
    //serial_write('\n');
  
    if(performingPowerFeed) return;
  
    if(motionFlags & ((count > 0)?JogUIPage_positiveDirectionLockout_Flag:JogUIPage_negativeDirectionLockout_Flag)) return;
    
    if(motionFlags & ((count > 0)?JogUIPage_negativeDirection_Flag:JogUIPage_positiveDirection_Flag))
    {
      if(sys.state & STATE_JOG) system_set_exec_state_flag(EXEC_MOTION_CANCEL);
      motionFlags &=~ (JogUIPage_positiveDirection_Flag | JogUIPage_negativeDirection_Flag| JogUIPage_didProbePositiveDirection_Flag | JogUIPage_didProbeNegativeDirection_Flag);
      return;
    }
    
    if(!UILineBufferIsAvailable()) return;

    const char *source = (gc_state.modal.units)?PSTR(UI_JOGGING_BASE_STRING_INCH):PSTR(UI_JOGGING_BASE_STRING_METRIC);
    char *cursor = UILineBuffer;
    char c;
    while ((c=pgm_read_byte(source++))) *cursor++ = c;
    *cursor++ = activeAxis;
    
    cursor = sPrintFloat(cursor, 10, activeIncrementValue * count, 4);
    *cursor = 0;

    UILineBufferState = UILineBufferState_ReadyForExecution;
    
    motionFlags |= ((count > 0)?JogUIPage_positiveDirection_Flag:JogUIPage_negativeDirection_Flag);
  }
    
  inline void JogUIPage::powerFeed(bool positiveDirection)
  {
    if(jogFeed < ((gc_state.modal.units)?(MINIMUM_FEED_RATE * 25.4):MINIMUM_FEED_RATE)) return;
    
    if(motionFlags & ((positiveDirection)?JogUIPage_positiveDirectionLockout_Flag:JogUIPage_negativeDirectionLockout_Flag)) return;
    
    if(motionFlags & ((positiveDirection)?JogUIPage_negativeDirection_Flag:JogUIPage_positiveDirection_Flag))
    {
      if(sys.state & STATE_JOG) system_set_exec_state_flag(EXEC_MOTION_CANCEL);
      motionFlags &=~ (JogUIPage_positiveDirection_Flag | JogUIPage_negativeDirection_Flag| JogUIPage_didProbePositiveDirection_Flag | JogUIPage_didProbeNegativeDirection_Flag);
      performingPowerFeed = false;
      return;
    }
    
    if(!UILineBufferIsAvailable()) return;

    const char *source = PSTR(UI_JOGGING_POWERFEED_BASE_STRING);
    char *cursor = UILineBuffer;
    char c;
    while ((c=pgm_read_byte(source++))) *cursor++ = c;
    cursor = sPrintFloat(cursor, 10, jogFeed, 1);
    
    *cursor++ = activeAxis;
    
    if(positiveDirection)
    {
    	float target;
    	switch(activeAxis)
    	{
    		case 'X':
    			target = -(settings.max_travel[X_AXIS]);
    		break;
    		
    		case 'Y':
    			target = -(settings.max_travel[Y_AXIS]);
    		break;
    		
    		case 'Z':
    			target = -(settings.max_travel[Z_AXIS]);
    		break;
    		
    		#if(N_AXIS > 3)
					case 'A':
						target = -(settings.max_travel[A_AXIS]);
					break;
    		#endif
    		
    		#if(N_AXIS > 4)
					case 'B':
						target = -(settings.max_travel[B_AXIS]);
					break;
    		#endif
    		
    		default:
          if(sys.state & STATE_JOG) system_set_exec_state_flag(EXEC_MOTION_CANCEL);
          performingPowerFeed = false;
    		  return;
    	}
    
    	cursor = sPrintFloat(cursor, 10, target, 4);
    }
    else
    {
    	*cursor++ = '0';
    }
    *cursor = 0;

    UILineBufferState = UILineBufferState_ReadyForExecution;
    
    motionFlags |= ((positiveDirection)?JogUIPage_positiveDirection_Flag:JogUIPage_negativeDirection_Flag);
    
    performingPowerFeed = true;
  }
    
  #ifndef USE_UI_ENCODER_B

    void JogUIPage::UIEncoderALeft(void)
    {
      jog(-1);
    }

    void JogUIPage::UIEncoderARight(void)
    {
      jog(1);
    }

  #else
    
    #ifndef USE_UI_ENCODER_B_ISR

      void JogUIPage::UIEncoderBLeft(void)
      {
        jog(-1);
      }

      void JogUIPage::UIEncoderBRight(void)
      {
        jog(1);
      }
    
    #else
      void JogUIPage::UIEncoderBCount(int8_t count)
      {
        jog(count);
      };
    #endif

  #endif

  void JogUIPage::UIEncoderButton1Pressed(void)
  {
    if(gc_state.modal.units)
    {
      // inch
      activeIncrement++;
      if(activeIncrement > UI_JOGGING_UPPER_INCH_RANGE) activeIncrement = UI_JOGGING_LOWER_INCH_RANGE;
    }
    else
    {
      // mm
      activeIncrement++;
      if(activeIncrement > UI_JOGGING_UPPER_METRIC_RANGE) activeIncrement = UI_JOGGING_LOWER_METRIC_RANGE;
    }
    updateActiveIncrementValue();
    
    drawStatusLine();
  }

  void JogUIPage::UIEncoderButton2Pressed(void)
  {
    goToUIPage(&statusUIPage);
  }

  void JogUIPage::keyPressed(uint8_t key)
  {
  	if(editingJogFeed)
  	{
  		editingKeyPressed(key);
  	}
  	else
  	{
			switch(key)
			{
				case '8':
				case 'X':
					if(sys.state & STATE_JOG)
						system_set_exec_state_flag(EXEC_MOTION_CANCEL);
					performingPowerFeed = false;
					activeAxis = 'X';
					motionFlags &=~ (JogUIPage_positiveDirectionLockout_Flag | JogUIPage_negativeDirectionLockout_Flag | JogUIPage_didProbePositiveDirection_Flag | JogUIPage_didProbeNegativeDirection_Flag);
					drawStatusLine();
				break;
			
				case '5':
				case 'Y':
					if(sys.state & STATE_JOG)
						system_set_exec_state_flag(EXEC_MOTION_CANCEL);
					performingPowerFeed = false;
					activeAxis = 'Y';
					motionFlags &=~ (JogUIPage_positiveDirectionLockout_Flag | JogUIPage_negativeDirectionLockout_Flag | JogUIPage_didProbePositiveDirection_Flag | JogUIPage_didProbeNegativeDirection_Flag);
					drawStatusLine();
				break;

				case '2':
				case 'Z':
					if(sys.state & STATE_JOG)
						system_set_exec_state_flag(EXEC_MOTION_CANCEL);
					performingPowerFeed = false;
					activeAxis = 'Z';
					motionFlags &=~ (JogUIPage_positiveDirectionLockout_Flag | JogUIPage_negativeDirectionLockout_Flag | JogUIPage_didProbePositiveDirection_Flag | JogUIPage_didProbeNegativeDirection_Flag);
					drawStatusLine();
				break;

				#if(N_AXIS > 3)
					case '.':
					case 'L':
						if(sys.state & STATE_JOG)
							system_set_exec_state_flag(EXEC_MOTION_CANCEL);
						performingPowerFeed = false;
						#if(N_AXIS > 4)
							activeAxis = (activeAxis == 'A')?'B':'A';
						#else
							activeAxis = 'A';
						#endif
						motionFlags &=~ (JogUIPage_positiveDirectionLockout_Flag | JogUIPage_negativeDirectionLockout_Flag | JogUIPage_didProbePositiveDirection_Flag | JogUIPage_didProbeNegativeDirection_Flag);
						drawStatusLine();
					break;
				#endif
			
				case '7':
				case 'G':
					if(performingPowerFeed)
					{
						if(sys.state & STATE_JOG)
							system_set_exec_state_flag(EXEC_MOTION_CANCEL);
						performingPowerFeed = false;
					}
					else
					{
						powerFeed(false);
					}
				break;
				
				case '4':
				case 'M':
					if(performingPowerFeed)
					{
						if(sys.state & STATE_JOG)
							system_set_exec_state_flag(EXEC_MOTION_CANCEL);
						performingPowerFeed = false;
					}
					else
					{
						powerFeed(true);
					}
				break;
			
				case '1':
				case 'F':
					if(sys.state & STATE_JOG)
						system_set_exec_state_flag(EXEC_MOTION_CANCEL); 
				
					editingJogFeed = true;
					
					if(jogFeed > 0.0)
					{
						editBufferState = sPrintFloat(editBuffer, 4, jogFeed, (gc_state.modal.units)?1:0) - editBuffer;
					}
					else
					{
						editBufferState = 0;
					}
					
					drawStatusLine();
				break;
			
				default:
					break;
			}
  	}
  }
	
	void JogUIPage::editingKeyPressed(uint8_t key)
	{
		if(editBufferState >= 4) return;
		if( ! (((key >= '0') && (key <= '9')) || (key == '.'))) return;
		
		editBuffer[editBufferState++] = key;
		
		drawStatusLine();
	}
	
	void JogUIPage::enterKeyPressed(void)
	{
		if(!editingJogFeed) return;
		
      if(editBufferState == 0)
      {
        jogFeed = 0.0;
      }
      else
      {
    		jogFeed = stringToFloat(editBuffer);
      }
    			
    editingJogFeed = false;
    
		drawStatusLine();
	}
	
	void JogUIPage::deleteKeyPressed(void)
	{
		if(editBufferState == 0) return;
		
		editBufferState--;
    
		drawStatusLine();
	}
	
  void JogUIPage::probePosKeyPressed(void)
  {
    if((sys.state != STATE_IDLE) || (!UILineBufferIsAvailable())) return;
    
    const char *source = PSTR("G38.3");
    char *cursor = UILineBuffer;
    char c;
    while ((c=pgm_read_byte(source++))) *cursor++ = c;
    *cursor++ = activeAxis;
    source= (gc_state.modal.units)?PSTR("1F1"):PSTR("25F25");
    *cursor = 0;
    
    UILineBufferState = UILineBufferState_ReadyForExecution;
    
    motionFlags |= (JogUIPage_positiveDirectionLockout_Flag  | JogUIPage_didProbePositiveDirection_Flag);
  }
  
  void JogUIPage::probeNegKeyPressed(void)
  {
    if((sys.state != STATE_IDLE) || (!UILineBufferIsAvailable())) return;
    
    const char *source = PSTR("G38.3");
    char *cursor = UILineBuffer;
    char c;
    while ((c=pgm_read_byte(source++))) *cursor++ = c;
    *cursor++ = activeAxis;
    source= (gc_state.modal.units)?PSTR("-1F1"):PSTR("-25F25");
    *cursor = 0;
    
    UILineBufferState = UILineBufferState_ReadyForExecution;
    
    motionFlags |= (JogUIPage_negativeDirectionLockout_Flag | JogUIPage_didProbeNegativeDirection_Flag);
  }

  void JogUIPage::smartZeroDiameter(float &position)
  {
  	float offset = get_tool_diameter(0) * 500.0;
  	if(gc_state.tool) offset += get_tool_diameter(gc_state.tool) * 500.0;
  	
		switch(motionFlags & (JogUIPage_didProbePositiveDirection_Flag | JogUIPage_didProbeNegativeDirection_Flag))
		{
			case JogUIPage_didProbePositiveDirection_Flag:
				{
					position += offset;
				}
				break;
			case JogUIPage_didProbeNegativeDirection_Flag:
				{
					position -= offset;
				}
				break;
		}
  }

  void JogUIPage::smartZeroLength(float &position)
  {
  	float offset = get_tool_length(0) * 1000.0;
  	
		switch(motionFlags & (JogUIPage_didProbePositiveDirection_Flag | JogUIPage_didProbeNegativeDirection_Flag))
		{
			case JogUIPage_didProbePositiveDirection_Flag:
				{
					position += offset;
				}
				break;
			case JogUIPage_didProbeNegativeDirection_Flag:
				{
					position -= offset;
				}
				break;
		}
  }

  void JogUIPage::partZeroKeyPressed(void)
  {
    if((sys.state != STATE_IDLE) || (!UILineBufferIsAvailable())) return;
    
    float position;
    
    switch(activeAxis)
    {
      case 'X':
        position = readAxisPosition_MachinePosition(0);
        #if ( TOOL_LENGTH_OFFSET_AXIS == 0 )
          position -= gc_state.tool_length_offset;
        #endif
        smartZeroDiameter(position);
        break;
        
      case 'Y':
        position = readAxisPosition_MachinePosition(1);
        #if ( TOOL_LENGTH_OFFSET_AXIS == 1 )
          position -= gc_state.tool_length_offset;
        #endif
        smartZeroDiameter(position);
        break;
        
      case 'Z':
        position = readAxisPosition_MachinePosition(2);
        #if ( TOOL_LENGTH_OFFSET_AXIS == 2 )
          position -= gc_state.tool_length_offset;
        #endif
        smartZeroLength(position);
        break;
        
      #if(N_AXIS > 3)
        case 'A':
          position = readAxisPosition_MachinePosition(3);
          break;
      #endif
        
      #if(N_AXIS > 4)
        case 'B':
          position = readAxisPosition_MachinePosition(4);
          break;
      #endif
      
      default:
        return;
    }
    
    const char *source = PSTR("G10L2P0");
    char *cursor = UILineBuffer;
    char c;
    while ((c=pgm_read_byte(source++))) *cursor++ = c;
    *cursor++ = activeAxis;
    cursor = sPrintFloat(cursor, 10, position, 4);    
    *cursor = 0;
    
    UILineBufferState = UILineBufferState_ReadyForExecution;
  }

  void JogUIPage::shiftedPartZeroKeyPressed(void)
  {
    if((sys.state != STATE_IDLE) || (!UILineBufferIsAvailable())) return;
    
    float position;
    
    switch(activeAxis)
    {
      case 'X':
        position = readAxisPosition_MachinePosition(0);
        #if ( TOOL_LENGTH_OFFSET_AXIS == 0 )
          position -= gc_state.tool_length_offset;
        #endif
        break;
        
      case 'Y':
        position = readAxisPosition_MachinePosition(1);
        #if ( TOOL_LENGTH_OFFSET_AXIS == 1 )
          position -= gc_state.tool_length_offset;
        #endif
        break;
        
      case 'Z':
        position = readAxisPosition_MachinePosition(2);
        #if ( TOOL_LENGTH_OFFSET_AXIS == 2 )
          position -= gc_state.tool_length_offset;
        #endif
        break;
        
      #if(N_AXIS > 3)
        case 'A':
          position = readAxisPosition_MachinePosition(3);
          break;
      #endif
        
      #if(N_AXIS > 4)
        case 'B':
          position = readAxisPosition_MachinePosition(4);
          break;
      #endif
      
      default:
        return;
    }
    
    const char *source = PSTR("G10L2P0");
    char *cursor = UILineBuffer;
    char c;
    while ((c=pgm_read_byte(source++))) *cursor++ = c;
    *cursor++ = activeAxis;
    cursor = sPrintFloat(cursor, 10, position, 4);    
    *cursor = 0;
    
    UILineBufferState = UILineBufferState_ReadyForExecution;
  }


  void JogUIPage::drawStatusLine(void)
  {
    lcd.setCursor(0,0);
    
    if(editingJogFeed)
    {
    	lcd.write_p(PSTR("=Jog: F "));
    	lcd.write(editBuffer, editBufferState);
    	lcd.write('_');
    	lcd.writeMultiple(' ', 10 - editBufferState);
    }
    else
    {
    	lcd.write_p(PSTR("=Jog: "));
			if(activeAxis)
			{
				lcd.write(activeAxis);
			
				switch(activeIncrement)
				{
					case 0: lcd.write_p(PSTR(" 0.0001")); break;
					case 1: lcd.write_p(PSTR(" 0.0010")); break;
					case 2: lcd.write_p(PSTR(" 0.0100")); break;
					case 3: lcd.write_p(PSTR(" 0.1000")); break;
					case 4: lcd.write_p(PSTR(" 1.0000")); break;
					case 5: lcd.write_p(PSTR(" 10.000")); break;
				}
			
				DROUpdatePhase = 0;	// Reset this because we're about to stomp on the buffer...
				DROTextBuffer[0] = 'F';
				sPrintFloatRightJustified(&DROTextBuffer[1], 4, jogFeed, 0);
				lcd.write(DROTextBuffer, 5);
			}
			else
			{
				lcd.write_p(PSTR("None.        "));
			}
		}
  }

  // MenuUIPage
    
    void MenuUIPage::showMenu(const UIMenuEntry *menu)
    {
      activeMenu = menu;
      goToUIPage(this);
    }

    void MenuUIPage::activate(void)
    {
      scrollOffset = 1;
      selectionOffset = 1;
    };
    
    void MenuUIPage::display(void)
    {
      lcd.setCursor(0,0);
      lcd.write(UI_MENU_PAGE_ICON_CHARACTER);
      lcd.write_p(&(activeMenu[0].text[0]), 19);
      drawMenuLines();
    }
    
    void MenuUIPage::drawMenuLines(void)
    {
      const UIMenuEntry *menuEntry = &(activeMenu[scrollOffset]);
      
      lcd.setCursor(0,1);
      lcd.write((scrollOffset == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
      lcd.write_p(&(menuEntry->text[0]), 19);
      
      menuEntry++;
      lcd.setCursor(0,2);
      lcd.write((scrollOffset + 1 == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
      lcd.write_p(&(menuEntry->text[0]), 19);
      
      menuEntry++;
      lcd.setCursor(0,3);
      lcd.write((scrollOffset + 2 == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
      lcd.write_p(&(menuEntry->text[0]), 19);
    }
        
    void MenuUIPage::UIEncoderALeft(void)
    {
      if(selectionOffset < 2) return;
      selectionOffset--;
      if(pgm_read_byte(&(activeMenu[selectionOffset].flags)) == UIMenuSeparatorFlag) selectionOffset--;
      if(scrollOffset > selectionOffset) scrollOffset = selectionOffset;
      drawMenuLines();
    }
    
    void MenuUIPage::UIEncoderARight(void)
    {
      if(pgm_read_byte(&(activeMenu[selectionOffset+1].flags)) == UIMenuEndFlag) return;
      selectionOffset++;
      if(pgm_read_byte(&(activeMenu[selectionOffset].flags)) == UIMenuSeparatorFlag) selectionOffset++;
      if((selectionOffset - scrollOffset) > 2) scrollOffset = selectionOffset - 2;
      drawMenuLines();
    }
    
    void MenuUIPage::UIEncoderButton1Pressed(void)
    {
      ((UIMenuFunction)pgm_read_ptr(&(activeMenu[selectionOffset].function)))();
    }

    void MenuUIPage::UIEncoderButton2Pressed(void)
    {
      goToUIPage(&statusUIPage);
    }

    void MenuUIPage::enterKeyPressed(void)
    {
      ((UIMenuFunction)pgm_read_ptr(&(activeMenu[selectionOffset].function)))();
    }

	// FormUIPage
	  void FormUIPage::showForm(UIFormFunction formFunction, const UIFormEntry *form, const char *title)
    {
    	activeFormFunction = formFunction;
      activeForm = form;
      activeFormTitle = title;
      goToUIPage(this);
    }

    void FormUIPage::activate(void)
    {
      scrollOffset = 0;
      selectionOffset = 0;
      loadEditBuffer();
    };
    
    void FormUIPage::display(void)
    {
      lcd.setCursor(0,0);
      lcd.write(UI_MENU_PAGE_ICON_CHARACTER);
      lcd.write_p(activeFormTitle, 18);
      drawFormLines();
    }
    
    void FormUIPage::drawFormLines(void)
    {
      const UIFormEntry *formEntry = &(activeForm[scrollOffset]);
      
      lcd.setCursor(0,1);
      if(pgm_read_byte(&(formEntry->flags)) == UIFormSeparatorFlag)
      {
        lcd.write_p(PSTR("    ----------     "));
      }
      else
      {
        lcd.write((scrollOffset == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        lcd.write_p(&(formEntry->text[0]), 9);
        drawFormLineValue(scrollOffset);
      }
      
      formEntry++;
      lcd.setCursor(0,2);
      if(pgm_read_byte(&(formEntry->flags)) == UIFormSeparatorFlag)
      {
        lcd.write_p(PSTR("    ----------     "));
      }
      else
      {
        lcd.write((scrollOffset + 1 == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        lcd.write_p(&(formEntry->text[0]), 9);
        drawFormLineValue(scrollOffset + 1);
      }
      
      formEntry++;
      lcd.setCursor(0,3);
      if(pgm_read_byte(&(formEntry->flags)) == UIFormSeparatorFlag)
      {
        lcd.write_p(PSTR("    ----------     "));
      }
      else
      {
        lcd.write((scrollOffset + 2 == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        lcd.write_p(&(formEntry->text[0]), 9);
        drawFormLineValue(scrollOffset + 2);
      }
    }
    
    void FormUIPage::drawFormLineValue(uint8_t index)
    {
    	uint8_t type = pgm_read_byte(&(activeForm[index].flags));
			if(type == UIFormEndFlag)
			{
				lcd.writeMultiple(' ', 10);
			  return;
			}
			
			type &= UIFormTypeMask;
			if(type == UIFormCheckboxFlag)
			{
				lcd.write_p(activeFormValues[index].b?PSTR(" [*]"):PSTR(" [ ]"));
				lcd.writeMultiple(' ', 6);
			}
			else
			{
				if(selectionOffset == index)
				{
					lcd.write(editBuffer, editBufferState);
					lcd.write('_');
					lcd.writeMultiple(' ', 9-editBufferState);
				}
				else
				{
				  if(activeFormValuesValid & (1UL << index))
				  {
            char buffer[10];
        
            switch(type)
            {
              case UIFormSignedFloatFlag:
              case UIFormUnsignedFloatFlag:
                {
                  char *end = sPrintFloat(buffer, 9, activeFormValues[index].f, 4);
                  *end = 0;
                  lcd.write_length(buffer, 9);
                  lcd.writeMultiple(' ', 10 - (end - buffer));
                }
                break;
            
              case UIFormSignedIntegerFlag:
                {
                  ltoa(activeFormValues[index].s, buffer, 10);
                  uint8_t padding = 10 - strlen(buffer);
                  lcd.write_length(buffer, 10);
                  lcd.writeMultiple(' ', padding);
                }
                break;
            
              case UIFormUnsignedIntegerFlag:
                {
                  ultoa(activeFormValues[index].u, buffer, 10);
                  uint8_t padding = 10 - strlen(buffer);
                  lcd.write_length(buffer, 10);
                  lcd.writeMultiple(' ', padding);
                }
                break;
            }
				  }
				  else
				  {
            lcd.writeMultiple(' ', 10);
				  }
				}
			}
    }
    
    void FormUIPage::loadEditBuffer(void)
    {
      uint8_t type = pgm_read_byte(&(activeForm[selectionOffset].flags));
			if(type == UIFormEndFlag) goto noEditBuffer;
			
			type &= UIFormTypeMask;
      if(activeFormValuesValid & (1UL << selectionOffset))
      {
        switch(type)
        {
          case UIFormSignedFloatFlag:
          case UIFormUnsignedFloatFlag:
            editBufferState = sPrintFloat(editBuffer, 9, activeFormValues[selectionOffset].f, 4) - editBuffer;
            editBuffer[editBufferState] = 0;
            return;
  
          case UIFormSignedIntegerFlag:
            ltoa(activeFormValues[selectionOffset].s, editBuffer, 10);
            editBufferState = strlen(editBuffer);
            return;
  
          case UIFormUnsignedIntegerFlag:
            ultoa(activeFormValues[selectionOffset].s, editBuffer, 10);
            editBufferState = strlen(editBuffer);
            return;
        }
      }
      
      noEditBuffer:
        editBuffer[0] = 0;
        editBufferState = 0;
    }
    
    void FormUIPage::acceptInput(void)
    {
      uint8_t type = pgm_read_byte(&(activeForm[selectionOffset].flags));
			if(type == UIFormEndFlag) return;
			type &= UIFormTypeMask;
      if(type == UIFormCheckboxFlag) return;
      
      if(editBufferState == 0)
      {
        activeFormValuesValid &=~ (1UL << selectionOffset);
        return;
      }
      
      editBuffer[editBufferState] = 0;
    	switch(type)
    	{
    		case UIFormSignedFloatFlag:
        case UIFormUnsignedFloatFlag:
    			activeFormValues[selectionOffset].f = stringToFloat(editBuffer);
    			break;
    		case UIFormSignedIntegerFlag:
    			activeFormValues[selectionOffset].s = atol(editBuffer);
    			break;
    		case UIFormUnsignedIntegerFlag:
    			activeFormValues[selectionOffset].u = atol(editBuffer);
    			break;
    	}
    	
      activeFormValuesValid |= (1UL << selectionOffset);      
    }
    
    void FormUIPage::UIEncoderALeft(void)
    {
      acceptInput();
      if(selectionOffset == 0) return;
      selectionOffset--;
      if(pgm_read_byte(&(activeForm[selectionOffset].flags)) == UIFormSeparatorFlag) selectionOffset--;
      if(scrollOffset > selectionOffset) scrollOffset = selectionOffset;
      loadEditBuffer();
      drawFormLines();
    }
    
    void FormUIPage::UIEncoderARight(void)
    {
      if(pgm_read_byte(&(activeForm[selectionOffset].flags)) == UIFormEndFlag) return;
      acceptInput();
      selectionOffset++;
      if(pgm_read_byte(&(activeForm[selectionOffset].flags)) == UIFormSeparatorFlag) selectionOffset++;
      if((selectionOffset - scrollOffset) > 2) scrollOffset = selectionOffset - 2;
      loadEditBuffer();
      drawFormLines();
    }
    
    void FormUIPage::UIEncoderButton1Pressed(void)
    {
      if((pgm_read_byte(&(activeForm[selectionOffset].flags)) & UIFormTypeMask) == UIFormCheckboxFlag)
      {
        activeFormValues[selectionOffset].b = ! activeFormValues[selectionOffset].b;
        lcd.setCursor(10, (selectionOffset - scrollOffset) + 1);
        drawFormLineValue(selectionOffset);
      }
    }

    void FormUIPage::UIEncoderButton2Pressed(void)
    {
      goToUIPage(&statusUIPage);
    }

    void FormUIPage::keyPressed(uint8_t key)
    {
      if(editBufferState >= 8) return;
      uint8_t type = pgm_read_byte(&(activeForm[selectionOffset].flags));
			if(type == UIFormEndFlag) return;
			type &= UIFormTypeMask;

    	switch(type)
    	{
    		case UIFormCheckboxFlag:
    		  break;
    		case UIFormSignedFloatFlag:
    		    if( ! (((key >= '0') && (key <= '9')) || (key == '-') || (key == '.'))) return;
    			break;
        case UIFormUnsignedFloatFlag:
    		    if( ! (((key >= '0') && (key <= '9')) || (key == '.'))) return;
    			break;
    		case UIFormSignedIntegerFlag:
    		    if( ! (((key >= '0') && (key <= '9')) || (key == '-'))) return;
    			break;
    		case UIFormUnsignedIntegerFlag:
    		    if( ! ((key >= '0') && (key <= '9'))) return;
    			break;
    	}
    	
    	editBuffer[editBufferState++] = key;
      
      lcd.setCursor(10, (selectionOffset - scrollOffset) + 1);
      drawFormLineValue(selectionOffset);
    }
    
    void FormUIPage::enterKeyPressed(void)
    {
      uint8_t type = pgm_read_byte(&(activeForm[selectionOffset].flags));
			if(type == UIFormEndFlag)
			{
			  activeFormFunction(activeFormValues);
			  return;
			}

      acceptInput();
      selectionOffset++;
      if(pgm_read_byte(&(activeForm[selectionOffset].flags)) == UIFormSeparatorFlag) selectionOffset++;
      if((selectionOffset - scrollOffset) > 2)
        scrollOffset = selectionOffset - 2;
      loadEditBuffer();
      drawFormLines();
    }
    
    void FormUIPage::deleteKeyPressed(void)
    {
      if(editBufferState == 0) return;
      
      editBufferState--;
    	
      lcd.setCursor(10, (selectionOffset - scrollOffset) + 1);
      drawFormLineValue(selectionOffset);
    }

  // FileDirectoryUIPage
  
    void FileDirectoryUIPage::drawDirectoryLines(void)
    {
      SD_Directory_Entry *directoryEntry = &SD_directory[scrollOffset];
      
      lcd.setCursor(0,1);
      if(SD_directory_count < 1)
      {
        lcd.writeMultiple(' ', 20);
      }
      else
      {
        lcd.write((scrollOffset == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        uint8_t padding = 16 - lcd.write_length(directoryEntry->name, 8);
        lcd.write_p(PSTR(".NC"));
        lcd.writeMultiple(' ', padding);
      }
      
      lcd.setCursor(0,2);
      if(SD_directory_count < 2)
      {
        lcd.writeMultiple(' ', 20);
      }
      else
      {
        lcd.write((scrollOffset + 1 == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        directoryEntry++;
        uint8_t padding = 16 - lcd.write_length(directoryEntry->name, 8);
        lcd.write_p(PSTR(".NC"));
        lcd.writeMultiple(' ', padding);
      }
      
      lcd.setCursor(0,3);
      if(SD_directory_count < 3)
      {
        lcd.writeMultiple(' ', 20);
      }
      else
      {
        lcd.write((scrollOffset + 2 == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        directoryEntry++;
        uint8_t padding = 16 - lcd.write_length(directoryEntry->name, 8);
        lcd.write_p(PSTR(".NC"));
        lcd.writeMultiple(' ', padding);
      }
    }
  
    void FileDirectoryUIPage::runSelectedFile(void)
    {
      char fileName[13];
      SD_Directory_Entry *directoryEntry = &SD_directory[selectionOffset];
      char *source = (char *)&(directoryEntry->name);
      char *destination = fileName;
      
      for (uint8_t count = 8; (*source) && (count); count-- )
        *destination++ = *source++;
      *destination++ = '.';
      *destination++ = 'N';
      *destination++ = 'C';
      *destination++ = 0;
     
      if(SD_open_filename(fileName))
      {
        goToUIPage(&statusUIPage);
      }
      else
      {
        UIShowError(PSTR("Can't open SD File. "));
      }
    }
  
    void FileDirectoryUIPage::activate(void)
    {
      scrollOffset = 0;
      selectionOffset = 0;
    };
    
    void FileDirectoryUIPage::display(void)
    {
      lcd.setCursor(0,0);
      lcd.write(UI_MENU_PAGE_ICON_CHARACTER);
      lcd.write_p(PSTR("Select File:      "));
      drawDirectoryLines();
    }
            
    void FileDirectoryUIPage::UIEncoderALeft(void)
    {
      if(selectionOffset < 1) return;
      selectionOffset--;
      if(scrollOffset > selectionOffset) scrollOffset = selectionOffset;
      drawDirectoryLines();
    }
    
    void FileDirectoryUIPage::UIEncoderARight(void)
    {
      if(selectionOffset >= (SD_directory_count - 1)) return;
      selectionOffset++;
      if((selectionOffset - scrollOffset) > 2) scrollOffset++;
      drawDirectoryLines();
    }
    
    void FileDirectoryUIPage::UIEncoderButton1Pressed(void)
    {
      runSelectedFile();
    }

    void FileDirectoryUIPage::UIEncoderButton2Pressed(void)
    {
      menuUIPage.showMenu((UIMenuEntry *)mainMenu);
    }

    void FileDirectoryUIPage::enterKeyPressed(void)
    {
      runSelectedFile();
    }

  // MDIUIPage
    
    void MDIUIPage::drawLines(void)
    {
      lcd.setCursor(0,2);
      
      if(UILineBufferState < 20)
      {
        // Only one line
        lcd.write(UILineBuffer, UILineBufferState);
        lcd.write('_');
        lcd.writeMultiple(' ', 19 - UILineBufferState);        
        lcd.setCursor(0,3);
        lcd.writeMultiple(' ', 20);        
      }
      else
      {
        // Two lines
        lcd.write(UILineBuffer, 20);
        lcd.setCursor(0,3);
        lcd.write(&UILineBuffer[20], UILineBufferState - 20);
        lcd.write('_');
        lcd.writeMultiple(' ', 39 - UILineBufferState);        
      }
    }
    
    void MDIUIPage::activate(void)
    {
      if( !UILineBufferIsAvailable() )
      {
        UIShowError(PSTR("  Busy - try again. "));
        return;
      }
      
      UILineBuffer[0] = 0;
      UILineBufferState = 0;
    }
    
    void MDIUIPage::deactivate(void)
    {
      UILineBufferState = 0;
    }
    
    void MDIUIPage::lineBufferExecuted(void)
    {
      UILineBuffer[0] = 0;
      UILineBufferState = 0;
      drawLines();
    }
    
    void MDIUIPage::display(void)
    {
      lcd.setCursor(0,0);
      lcd.write_p(PSTR("=MDI               "));
      lcd.setCursor(0,1);
      lcd.writeMultiple(' ', 20);
      drawLines();
    }

    void MDIUIPage::UIEncoderButton2Pressed(void)
    {
      goToUIPage(&statusUIPage);
    }

    void MDIUIPage::keyPressed(uint8_t key)
    {
      if(UILineBufferState < 39)
      {
        if((key == 'L') && UILineBufferState)
        {
          switch(UILineBuffer[UILineBufferState-1])
          {
            case 'L':
              UILineBuffer[UILineBufferState-1] = 'P';
              break;
            
            #if(N_AXIS > 3)
              case 'P':
                UILineBuffer[UILineBufferState-1] = 'A';
                break;
            #endif
            
            #if(N_AXIS > 4)
              case 'A':
                UILineBuffer[UILineBufferState-1] = 'B';
                break;
            #endif
            
            default:
              UILineBuffer[UILineBufferState++] = key;
              UILineBuffer[UILineBufferState] = 0;
              break;
          }
        }
        else
        {
          UILineBuffer[UILineBufferState++] = key;
          UILineBuffer[UILineBufferState] = 0;
        }
      
        drawLines();
      }
    }
    
    void MDIUIPage::enterKeyPressed(void)
    {
      if(UILineBufferState)
      {
        if(UILineBufferState < 39) UILineBufferState = UILineBufferState_ReadyForExecution;
      }
      else
      {
        goToUIPage(&statusUIPage);
      }
    }
    
    void MDIUIPage::shiftedEnterKeyPressed(void)
    {
    	if((!rpnCalculatorUIPage.valueAvailable()) || (UILineBufferState > 37)) return;
    	float value = rpnCalculatorUIPage.popValue();
    	UILineBufferState = sPrintFloat(&UILineBuffer[UILineBufferState], 40 - UILineBufferState, value, 4) - UILineBuffer;
      drawLines();
    }
    
    void MDIUIPage::deleteKeyPressed(void)
    {
      if((UILineBufferState > 0) && (UILineBufferState < 40))
      {
        UILineBufferState--;
        UILineBuffer[UILineBufferState] = 0;
        drawLines();
      }
    }

// ToolDataUIPage

    void ToolDataUIPage::activate(void)
    {
      scrollOffset = 0;
      selectionOffset = 0;
      loadEditBuffer();
    };
    
    void ToolDataUIPage::display(void)
    {
      lcd.setCursor(0,0);
      lcd.write(UI_MENU_PAGE_ICON_CHARACTER);
      lcd.write_p(PSTR("Tool Data:        "));
      drawToolDataLines();
    }
    
    void ToolDataUIPage::drawToolLengthLine(uint8_t tool, bool editing)
    {
      char buffer[19];
      if(tool)
      {
				if(tool < 10)
				{
					buffer[0] = ' ';
					buffer[1] = '0' + tool;
				}
				else
				{
					itoa(tool, buffer, 10);
				}
			}
			else
			{
				buffer[0] = 'P';
				buffer[1] = 'b';
			}
      memcpy_P(&buffer[2], PSTR(" Len: "), 6);
      
      if(editing)
      {
        lcd.write(buffer, 8);
				lcd.write(editBuffer, editBufferState);
				lcd.write('_');
				lcd.writeMultiple(' ', 10-editBufferState);
      }
      else
      {
        char *cursor = sPrintFloat(&buffer[8], 9, get_tool_length(tool) / 1000.0, 3);
        while(cursor < (buffer + 19)) *cursor++ = ' '; 
        lcd.write(buffer, 19);
      }
    }
    
    void ToolDataUIPage::drawToolDiameterLine(uint8_t tool, bool editing)
    {
      char buffer[19];
      if(tool)
      {
				if(tool < 10)
				{
					buffer[0] = ' ';
					buffer[1] = '0' + tool;
				}
				else
				{
					itoa(tool, buffer, 10);
				}
			}
			else
			{
				buffer[0] = 'P';
				buffer[1] = 'b';
			}
      memcpy_P(&buffer[2], PSTR(" Dia: "), 6);
      
      if(editing)
      {
        lcd.write(buffer, 8);
				lcd.write(editBuffer, editBufferState);
				lcd.write('_');
				lcd.writeMultiple(' ', 10-editBufferState);
      }
      else
      {
        char *cursor = sPrintFloat(&buffer[8], 9, get_tool_diameter(tool) / 1000.0, 3);
        while(cursor < (buffer + 19)) *cursor++ = ' '; 
        lcd.write(buffer, 19);
      }
    }
    
    void ToolDataUIPage::drawToolDataLines(void)
    {
      uint8_t tool = scrollOffset >> 1;
      
      if(scrollOffset & 0x01)
      {
        lcd.setCursor(0,1);
        lcd.write((scrollOffset == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        drawToolDiameterLine(tool, (scrollOffset == selectionOffset));
        
        lcd.setCursor(0,2);
        lcd.write((scrollOffset + 1 == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        drawToolLengthLine(tool + 1, (scrollOffset + 1 == selectionOffset));
        
        lcd.setCursor(0,3);
        lcd.write((scrollOffset + 2 == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        drawToolDiameterLine(tool + 1, (scrollOffset + 2 == selectionOffset));
      }
      else
      {
        lcd.setCursor(0,1);
        lcd.write((scrollOffset == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        drawToolLengthLine(tool, (scrollOffset == selectionOffset));
        
        lcd.setCursor(0,2);
        lcd.write((scrollOffset + 1 == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        drawToolDiameterLine(tool, (scrollOffset + 1 == selectionOffset));
        
        lcd.setCursor(0,3);
        lcd.write((scrollOffset + 2 == selectionOffset)?UI_MENU_INDICATOR_CHARACTER:' ');
        drawToolLengthLine(tool + 1, (scrollOffset + 2 == selectionOffset));
      }
    }
    
    void ToolDataUIPage::loadEditBuffer(void)
    {
      uint32_t value = (selectionOffset & 0x01)?get_tool_diameter(selectionOffset >> 1):get_tool_length(selectionOffset >> 1);
      if(value > 0)
      {
        editBufferState = sPrintFloat(editBuffer, 6, value / 1000.0, 3) - editBuffer;
      }
      else
      {
        editBufferState = 0;
      }
    }
        
    void ToolDataUIPage::acceptInput(void)
    {
      editBuffer[editBufferState] = 0;
      uint32_t parsedInput = round(stringToFloat(editBuffer) * 1000.0);
      if(selectionOffset & 0x01)
      {
        uint32_t storedValue = get_tool_diameter(selectionOffset >> 1);
        if (parsedInput == storedValue) return; // Check keeps from burning out the eeprom if nothing is changing.
        store_tool_diameter(selectionOffset >> 1, parsedInput);
      }
      else
      {
        uint32_t storedValue = get_tool_length(selectionOffset >> 1);
        if (parsedInput == storedValue) return; // Check keeps from burning out the eeprom if nothing is changing.
        store_tool_length(selectionOffset >> 1, parsedInput);
      }
    }
    
    void ToolDataUIPage::UIEncoderALeft(void)
    {
      acceptInput();
      if(selectionOffset == 0) return;
      selectionOffset--;
      if(scrollOffset > selectionOffset) scrollOffset = selectionOffset;
      loadEditBuffer();
      drawToolDataLines();
    }
    
    void ToolDataUIPage::UIEncoderARight(void)
    {
      acceptInput();
      if(selectionOffset >= (MAX_TOOL_NUMBER * 2)) return;
      selectionOffset++;
      if((selectionOffset - scrollOffset) > 2) scrollOffset = selectionOffset - 2;
      loadEditBuffer();
      drawToolDataLines();
    }
    
    void ToolDataUIPage::UIEncoderButton2Pressed(void)
    {
      goToUIPage(&statusUIPage);
    }

    void ToolDataUIPage::keyPressed(uint8_t key)
    {
      if(editBufferState >= 9) return;
      if(selectionOffset & 0x01)
      {
    		if( ! (((key >= '0') && (key <= '9')) || (key == '.'))) return;
    	}
    	else
    	{
    		if( ! (((key >= '0') && (key <= '9')) || (key == '-') || (key == '.'))) return;
    	}
    	
    	editBuffer[editBufferState++] = key;
      
			lcd.setCursor(9, (selectionOffset - scrollOffset) + 1);
			lcd.write(editBuffer, editBufferState);
			lcd.write('_');
			lcd.writeMultiple(' ', 10-editBufferState);
    }
    
    void ToolDataUIPage::enterKeyPressed(void)
    {
      acceptInput();
      if(selectionOffset >= (MAX_TOOL_NUMBER * 2)) return;
      selectionOffset++;
      if((selectionOffset - scrollOffset) > 2)
        scrollOffset = selectionOffset - 2;
      loadEditBuffer();
      drawToolDataLines();
    }
    
    void ToolDataUIPage::deleteKeyPressed(void)
    {
      if(editBufferState == 0) return;
      
      editBufferState--;
    	
			lcd.setCursor(9, (selectionOffset - scrollOffset) + 1);
			lcd.write(editBuffer, editBufferState);
			lcd.write('_');
			lcd.writeMultiple(' ', 10-editBufferState);
    }
    
	// RPNCalculatorUIPage
	
    void RPNCalculatorUIPage::activate(void)
    {
			editing = false;
			editBufferState = 0;
			scrollOffset = 0;
			selectionOffset = -1;
    }

    void RPNCalculatorUIPage::display(void)
    {
    	lcd.setCursor(0,0);
    	lcd.write_p(PSTR("=RPN Calculator:   "));
    	
    	drawStack();
    }
    
    inline void RPNCalculatorUIPage::drawEditLine(void)
    {
			lcd.setCursor(0,3);
			lcd.write(editBuffer, editBufferState);
			lcd.write('_');
			lcd.writeMultiple(' ', 19-editBufferState);
    }

    void RPNCalculatorUIPage::drawStackLine(uint8_t level)
    {
    	lcd.write((selectionOffset == level)?UI_MENU_INDICATOR_CHARACTER:' ');
    	lcd.write('0' + level);
    	lcd.write(':');
    	lcd.writeMultiple(' ', 7);
    	if(level < stackTop)
    	{
				sPrintFloatRightJustified(DROTextBuffer, 10, stack[level], 4);
				lcd.write(DROTextBuffer, 10);
    	}
    	else
    	{
    		lcd.writeMultiple(' ', 10);
    	}
    }

    void RPNCalculatorUIPage::drawStack(void)
    {
    	if(editing)
    	{
    		lcd.setCursor(0,1);
    		drawStackLine(1);
    		
    		lcd.setCursor(0,2);
    		drawStackLine(0);
    		
        drawEditLine();
    	}
    	else
    	{
    		lcd.setCursor(0,1);
    		drawStackLine(scrollOffset + 2);
    		
    		lcd.setCursor(0,2);
    		drawStackLine(scrollOffset + 1);
    		
    		lcd.setCursor(0,3);
    		drawStackLine(scrollOffset);
    	}
    }

		inline void RPNCalculatorUIPage::push(float value)
		{
			for(uint8_t cursor = min(stackTop, 9); cursor > 0; cursor--) stack[cursor] = stack[cursor - 1];
			stack[0] = value;
			if(stackTop < 10) stackTop++;
		}
				
    inline void RPNCalculatorUIPage::dropStackLevel(uint8_t level)
    {
    	if(level >= stackTop) return;
			stackTop--;
    	if(level < stackTop) for(uint8_t cursor = level; cursor < stackTop; cursor++) stack[cursor] = stack[cursor + 1];
    }
    
		inline void RPNCalculatorUIPage::acceptInput()
		{
			if(!editing) return;
      editBuffer[editBufferState] = 0;
			if(editBufferState > 0) push(stringToFloat(editBuffer));
			editing = false;
			editBufferState = 0;
		}
		
    void RPNCalculatorUIPage::UIEncoderALeft(void)
    {
			acceptInput();
						
      if((selectionOffset < (stackTop - 1)) && (stackTop > 0))
      {
        selectionOffset++;
        if((selectionOffset - scrollOffset) > 2) scrollOffset = selectionOffset - 2;
			};

			drawStack();
    }

    void RPNCalculatorUIPage::UIEncoderARight(void)
    {
			acceptInput();
			
      if(selectionOffset >= 0)
      {
        selectionOffset--;
        if(scrollOffset > selectionOffset) scrollOffset = max(selectionOffset, 0);
			}
			
			drawStack();
    }

    void RPNCalculatorUIPage::UIEncoderButton1Pressed(void)
    {
    	if((selectionOffset < 0)) return;
    	
			editBufferState = sPrintFloat(editBuffer, 10, stack[selectionOffset], 4) - editBuffer;
			editing = true;
			scrollOffset = 0;
			selectionOffset = -1;
			
			drawStack();
    }

    void RPNCalculatorUIPage::UIEncoderButton2Pressed(void)
    {
      goToUIPage(&statusUIPage);
    }

    void RPNCalculatorUIPage::keyPressed(uint8_t key)
    {
    	switch(key)
    	{
    		case 'G': // Add
    			acceptInput();
    			if(stackTop > 0)
    			{
    				stack[0] += stack[1];
    				dropStackLevel(1);
    			}
					scrollOffset = 0;
					selectionOffset = -1;
    			drawStack();
    		break;
    		
    		case 'M': // Subtract
    			acceptInput();
    			if(stackTop > 0)
    			{
    				stack[0] = stack[1] - stack[0];
    				dropStackLevel(1);
    			}
					scrollOffset = 0;
					selectionOffset = -1;
    			drawStack();
    		break;
    		
    		case 'F': // Multiply
    			acceptInput();
    			if(stackTop > 0)
    			{
    				stack[0] *= stack[1];
    				dropStackLevel(1);
    			}
					scrollOffset = 0;
					selectionOffset = -1;
    			drawStack();
    		break;
    		
    		case 'S': // Divide
    			acceptInput();
    			if(stackTop > 0)
    			{
    				stack[0] = stack[1] / stack[0];
    				dropStackLevel(1);
    			}
					scrollOffset = 0;
					selectionOffset = -1;
    			drawStack();
    		break;
    		
    		case 'X':
    		{
					float value = readAxisPosition_WorkPosition(X_AXIS);
					editBufferState = sPrintFloat(editBuffer, 10, value, 4) - editBuffer;
					editing = true;
					scrollOffset = 0;
					selectionOffset = -1;
			
					drawStack();
    		}
    		break;
    		
    		case 'Y':
    		{
					float value = readAxisPosition_WorkPosition(Y_AXIS);
					editBufferState = sPrintFloat(editBuffer, 10, value, 4) - editBuffer;
					editing = true;
					scrollOffset = 0;
					selectionOffset = -1;
			
					drawStack();
    		}
    		break;
    		
    		case 'Z':
    		{
					float value = readAxisPosition_WorkPosition(Z_AXIS);
					editBufferState = sPrintFloat(editBuffer, 10, value, 4) - editBuffer;
					editing = true;
					scrollOffset = 0;
					selectionOffset = -1;
			
					drawStack();
    		}
    		break;
    		
    		default:
					if(((key >= '0') && (key <= '9')) || (key == '-') || (key == '.')) editBuffer[editBufferState++] = key;
    			if(!editing)
    			{
    				editing = true;
						scrollOffset = 0;
						selectionOffset = -1;
    				drawStack();
    				return;
    			}
          drawEditLine();
    	}    	
    }

    void RPNCalculatorUIPage::enterKeyPressed(void)
    {
    	if(editing)
    	{
        acceptInput();
    	}
    	else
    	{
    	  if((stackTop > 0) && (selectionOffset == -1)) push(stack[0]);
    	}
    	
      drawStack();
    }

    void RPNCalculatorUIPage::deleteKeyPressed(void)
    {
    	if(editing)
    	{
				if(editBufferState == 0) return;
				editBufferState--;
				drawEditLine();
    	}
    	else
    	{
				dropStackLevel(max(selectionOffset, 0));
				if(selectionOffset >= stackTop) selectionOffset = stackTop - 1;
        if(scrollOffset > selectionOffset) scrollOffset = max(selectionOffset, 0);
    		drawStack();
    	}
    }

    void RPNCalculatorUIPage::partZeroKeyPressed(void)
    {
    }

    void RPNCalculatorUIPage::shiftedPartZeroKeyPressed(void)
    {
    }

#endif