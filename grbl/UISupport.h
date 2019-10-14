#ifndef UISUPPORT_H
#define UISUPPORT_H

  #include "grbl.h"

  #ifdef USE_UI_SUPPORT

    #ifdef __cplusplus	
      extern "C"{
    #endif
      
      #define UILineBufferSize                        (40)
      #define UILineBufferState_ReadyForExecution     (0xFE)
      #define UILineBufferState_Busy                  (0xFF)
      
      extern char UILineBuffer[UILineBufferSize];
      extern uint8_t UILineBufferState;
      void UIBufferExecuted(void);  // Called to notify the UI subsystem that the UILineBuffer has been executed by protocol.c
      static inline bool UILineBufferIsAvailable() { return UILineBufferState < UILineBufferSize; }
      // UILineBufferState values < UILineBufferSize indicate the amount of text in the line buffer,
      // and that the buffer is available for use by the UI. UILineBufferState_ReadyForExecution indicates
      // that the buffer is full of a command, is null-terminated, and is ready to be executed by protocol.c
      // UILineBufferState_Busy indicates that the buffer is currently being executed and is not available 
      // for the UI to use.
      
      #define UIExecuteGCode_Option_Default           (0x00)
      #define UIExecuteGCode_Option_StartOfSequence   (0x01)
      #define UIExecuteGCode_Option_ResetOnUserAbort  (0x02)
      #define UIExecuteGCode_Option_Synchronous       (0x04)

      extern uint8_t UIExecuteGCode(char *buffer, uint8_t options);
      
      void UISetup(void);
      void UITask(void);
      void UIStatusNeedsUpdate(void);
      
      
      typedef void (*UIDialogResponseFunction)(void);
      void UIShowDialogPage(const char *message, UIDialogResponseFunction okFunction, UIDialogResponseFunction cancelFunction); // String must be located in program memory.
      
      void UIShowError(const char *errorMessage); // String must be located in program memory.


    #ifdef __cplusplus
      } // extern "C"
      
      class AbstractUIPage
      {
        public:
          virtual void activate(void);
          virtual void deactivate(void);
    
          virtual void lineBufferExecuted(void);                      // Notification that the UILineBuffer has been executed by protocol.c
    
          virtual void display(void);                                 // Redraws the whole page.
          virtual void update(void);                                  // Selectively redraws any sections of the page that needs a redraw.
  
          // User interface event handlers
            virtual void UIEncoderALeft(void);
            virtual void UIEncoderARight(void);

            #ifdef USE_UI_ENCODER_B
              #ifndef USE_UI_ENCODER_B_ISR
                virtual void UIEncoderBLeft(void);
                virtual void UIEncoderBRight(void);
              #else
                virtual void UIEncoderBCount(int8_t);
              #endif
            #endif
  
            virtual void UIEncoderButton1Pressed(void);
            virtual void UIEncoderButton2Pressed(void);

            virtual void keyPressed(uint8_t key);
            virtual void enterKeyPressed(void);
            virtual void shiftedEnterKeyPressed(void);
            virtual void deleteKeyPressed(void);
            virtual void probePosKeyPressed(void);
            virtual void probeNegKeyPressed(void);
            virtual void partZeroKeyPressed(void);
            virtual void shiftedPartZeroKeyPressed(void);
      };

      extern AbstractUIPage *activeUIPage;
      static inline void goToUIPage(AbstractUIPage *uiPage)
      {
        activeUIPage->deactivate();
        activeUIPage = uiPage;
        activeUIPage->activate();
        activeUIPage->display(); 
      }
      
      class DialogUIPage : public AbstractUIPage
			{
				public:
					const char *dialogMessage;   // String must be located in program memory.
					UIDialogResponseFunction okFunction;
					UIDialogResponseFunction cancelFunction;
		
					virtual void display(void);
		
					virtual void enterKeyPressed(void);
					virtual void UIEncoderButton2Pressed(void);
			};

			class ErrorUIPage : public AbstractUIPage
			{
				public:
					const char *errorMessage;   // String must be located in program memory.
					virtual void display(void);
		
					virtual void UIEncoderButton1Pressed(void);
					virtual void UIEncoderButton2Pressed(void);
		
					virtual void enterKeyPressed(void);
			};

			class StatusUIPage : public AbstractUIPage
			{
				private:
					int8_t subPage;
					uint8_t runIndicatorCounter;
					int8_t runIndicatorPhase;
				public:
          virtual void activate(void);
					
					virtual void display(void);
					virtual void update(void);
					
					virtual void UIEncoderALeft(void);
					virtual void UIEncoderARight(void);
          virtual void UIEncoderButton1Pressed(void);
				private:
					void drawStatusLine(void);
			};

			class JogUIPage : public AbstractUIPage
			{
				private:
					char activeAxis;
					uint8_t motionFlags;
					int8_t activeIncrement;
					float activeIncrementValue;
					bool performingPowerFeed;
					float jogFeed;
					bool editingJogFeed;
							
					inline void updateActiveIncrementValue(void);

					void jog(int8_t count);
					void powerFeed(bool positiveDirection);
					void smartZeroDiameter(float &position);
					void smartZeroLength(float &position);
					
					void editingKeyPressed(uint8_t key);
					
				public:
					virtual void activate(void);
					virtual void deactivate(void);
	
					virtual void display(void);
					virtual void update(void);
		
					#ifndef USE_UI_ENCODER_B
						virtual void UIEncoderALeft(void);
						virtual void UIEncoderARight(void);
					#else
						#ifndef USE_UI_ENCODER_B_ISR
							virtual void UIEncoderBLeft(void);
							virtual void UIEncoderBRight(void);
						#else
							virtual void UIEncoderBCount(int8_t);
						#endif
					#endif

					virtual void UIEncoderButton1Pressed(void);
					virtual void UIEncoderButton2Pressed(void);

					virtual void keyPressed(uint8_t key);
					virtual void enterKeyPressed(void);
					virtual void deleteKeyPressed(void);
					virtual void probePosKeyPressed(void);
					virtual void probeNegKeyPressed(void);
					virtual void partZeroKeyPressed(void);
					virtual void shiftedPartZeroKeyPressed(void);
		
				private:
					void drawStatusLine(void);
			};

      // Menu Definition Structures
      #define UIMenuSeparatorFlag     0x01
      #define UIMenuEndFlag           0x80
      #define UIMenuSeparator       { 0x01, NULL, "    ----------     " }
      #define UIMenuEnd             { 0x80, NULL, "" }

      typedef void (*UIMenuFunction)(void);
      typedef struct
      {
        uint8_t flags;
        UIMenuFunction function;
        char text[20];
      } UIMenuEntry;
      #define UIMenu_h extern const UIMenuEntry
      #define UIMenu const UIMenuEntry __attribute__((__progmem__, used))
      
      class MenuUIPage : public AbstractUIPage
      {
        protected:
          uint8_t scrollOffset;
          uint8_t selectionOffset;
          const UIMenuEntry *activeMenu;
    
          void drawMenuLines(void);
    
        public:
          void showMenu(const UIMenuEntry *menu);
          virtual void activate(void);
  
          virtual void display(void);

          virtual void UIEncoderALeft(void);
          virtual void UIEncoderARight(void);
    
          virtual void UIEncoderButton1Pressed(void);
          virtual void UIEncoderButton2Pressed(void);
    
          virtual void enterKeyPressed(void);
      };
  
      #define UIFormTypeMask          		0x0F
      #define UIFormSeparatorFlag     		0x01
      #define UIFormSignedFloatFlag       0x02
      #define UIFormUnsignedFloatFlag     0x03
      #define UIFormSignedIntegerFlag     0x04
      #define UIFormUnsignedIntegerFlag   0x05
      #define UIFormCheckboxFlag      		0x06
      #define UIFormProbeableFlag     		0x10      
      #define UIFormEndFlag           		0x80
      
      #define UIFormSeparator       		{ 0x01, "" }

      typedef struct
      {
        uint8_t flags;
        char text[10];
      } UIFormEntry;
      #define UIForm_h extern const UIFormEntry
      #define UIForm const UIFormEntry __attribute__((__progmem__, used))

      typedef union
      {
        bool b;
        int32_t s;
        uint32_t u;
        float f;
      } UIFormValue;
      
      typedef void (*UIFormFunction)(UIFormValue *);
      
      #define UIFormMaxEntries  (32)
      class FormUIPage : public AbstractUIPage
      {
        private:
        	uint32_t activeFormValuesValid;   // Bitfield indicating which form entries have valid data in them.
          UIFormValue activeFormValues[UIFormMaxEntries];
					const UIFormEntry *activeForm;	// Must be in program memory space.
					const char *activeFormTitle;		// Must be in program memory space.
          UIFormFunction activeFormFunction;
          uint8_t scrollOffset;
          uint8_t selectionOffset;
          
          void drawFormLines(void);
          void drawFormLineValue(uint8_t index);
          void loadEditBuffer(void);
          void acceptInput(void);

    
        public:
          void showForm(UIFormFunction formFunction, const UIFormEntry *form, const char *title);	// All arguments must be in program memory space.
					inline void clearAllFormValues(void) { activeFormValuesValid = 0; };
					inline void clearFormValue(uint8_t index) { activeFormValuesValid &=~ (1UL << index); };
					inline void setFormValue(uint8_t index, UIFormValue value) { activeFormValues[index] = value; activeFormValuesValid |= (1UL << index); };
					
					virtual void activate(void);
					
          virtual void display(void);
					
					virtual void UIEncoderALeft(void);
					virtual void UIEncoderARight(void);

					virtual void UIEncoderButton1Pressed(void);
					virtual void UIEncoderButton2Pressed(void);

					virtual void keyPressed(uint8_t key);
					virtual void enterKeyPressed(void);
					virtual void deleteKeyPressed(void);
      };
      
      class FileDirectoryUIPage : public AbstractUIPage
			{
				protected:
					uint8_t scrollOffset;
					uint8_t selectionOffset;
		
					void drawDirectoryLines(void);
					void runSelectedFile(void);

				public:
					virtual void activate(void);

					virtual void display(void);
		
					virtual void UIEncoderALeft(void);
					virtual void UIEncoderARight(void);
		
					virtual void UIEncoderButton1Pressed(void);
					virtual void UIEncoderButton2Pressed(void);
		
					virtual void enterKeyPressed(void);
			};

			class MDIUIPage : public AbstractUIPage
			{
				private:
					void drawLines(void);
		
				public:
					virtual void activate(void);
					virtual void deactivate(void);
		
					virtual void lineBufferExecuted(void);
		
					virtual void display(void);
		
					virtual void UIEncoderButton2Pressed(void);
		
					virtual void keyPressed(uint8_t key);
					virtual void enterKeyPressed(void);
					virtual void shiftedEnterKeyPressed(void);
					virtual void deleteKeyPressed(void);
			};

      class ToolDataUIPage : public AbstractUIPage
      {
        private:
          uint8_t scrollOffset;
          uint8_t selectionOffset;
          
					void drawToolLengthLine(uint8_t tool, bool editing);
					void drawToolDiameterLine(uint8_t tool, bool editing);
          
          void drawToolDataLines(void);
          void loadEditBuffer(void);
          void acceptInput(void);

    
        public:					
					virtual void activate(void);
					
          virtual void display(void);
					
					virtual void UIEncoderALeft(void);
					virtual void UIEncoderARight(void);

					virtual void UIEncoderButton2Pressed(void);

					virtual void keyPressed(uint8_t key);
					virtual void enterKeyPressed(void);
					virtual void deleteKeyPressed(void);
      };
      
      class RPNCalculatorUIPage : public AbstractUIPage
      {
        private:
          int8_t scrollOffset;
          int8_t selectionOffset;
          int8_t stackTop;
          bool editing;
          float stack[10];
      
    			void drawEditLine(void);
      		void drawStackLine(uint8_t level);
					void drawStack(void);
					void push(float value);
      		void dropStackLevel(uint8_t level);
      		void acceptInput(void);
      
        public:
        	RPNCalculatorUIPage() { stackTop = 0; editing = false; };
        
          virtual void activate(void);
    
          virtual void display(void);
  
          // User interface event handlers
            virtual void UIEncoderALeft(void);
            virtual void UIEncoderARight(void);

            virtual void UIEncoderButton1Pressed(void);
            virtual void UIEncoderButton2Pressed(void);

            virtual void keyPressed(uint8_t key);
            virtual void enterKeyPressed(void);
            virtual void deleteKeyPressed(void);
            virtual void partZeroKeyPressed(void);
            virtual void shiftedPartZeroKeyPressed(void);
            
          // Accessors
          	inline bool valueAvailable(void) { return stackTop > 0; }
          	inline float popValue(void) { float value = stack[0]; dropStackLevel(0); return value; }	// Calling this when valueAvailable() is false is an error.
      };

      
      extern DialogUIPage dialogUIPage;
			extern ErrorUIPage errorUIPage;
			extern StatusUIPage statusUIPage;
			extern JogUIPage jogUIPage;
			extern MenuUIPage menuUIPage;
			extern FormUIPage formUIPage;
			extern FileDirectoryUIPage fileDirectoryUIPage;
			extern MDIUIPage mdiUIPage;
			extern ToolDataUIPage toolDataUIPage;
			extern RPNCalculatorUIPage rpnCalculatorUIPage;

    #endif

  #else
    #define UISetup()
    #define UITask()
    #define UIStatusNeedsUpdate()
  #endif
  
#endif