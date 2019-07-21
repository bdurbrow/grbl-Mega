#include "UIMenus.h"
#include "UIConversational.h"

void goToSDFileDirectoryUIPage(void);
void goToFacingOperationUIPage(void);
void goToSlotingOperationUIPage(void);
void goToDrillingOperationUIPage(void);
void goToDrillPatternOperationUIPage(void);
void goToRPNCalculatorUIPage(void);
void goToSystemMenu(void);

void homeAllAxis(void);
void clearAlarm(void);
void softReset(void);
void goToToolDataUIPage(void);
void goToAxisSettingsUIPage(void);
void goToSystemSettingsUIPage(void);

UIMenu mainMenu[] =
{
  { 0, NULL,                              "Main Menu:         "     },
  { 0, goToSDFileDirectoryUIPage,         "SD Card           \x01"  },
  UIMenuSeparator,
  { 0, goToFacingOperationUIPage,         "Face               "     },
  { 0, goToSlotingOperationUIPage,        "Slot               "     },
  { 0, goToDrillingOperationUIPage,       "Drill Single Hole  "     },
  { 0, goToDrillPatternOperationUIPage,   "Bolt Hole Circle   "     },
  UIMenuSeparator,
  { 0, goToRPNCalculatorUIPage,           "RPN Calculator     "     },
  { 0, goToSystemMenu,                    "System            \x01"  },
  UIMenuEnd
};

UIMenu systemMenu[] =
{
  { 0, NULL,                              "System Menu:       "     },
  { 0, homeAllAxis,                       "Home All Axis      "     },
  { 0, clearAlarm,                        "Clear Alarm        "     },
  { 0, softReset,                         "Soft Reset         "     },
  UIMenuSeparator,
  { 0, goToToolDataUIPage,                "Edit Tool Data    \x01"  },
  { 0, goToAxisSettingsUIPage,            "Edit Axis Settings\x01"  },
  { 0, goToSystemSettingsUIPage,          "Edit Sys Settings \x01"  },
  UIMenuEnd
};

// Main Menu/Dialog functions

  void cancelFileDialogOK()
  {
    SD_close();
    mc_reset();
    menuUIPage.showMenu((UIMenuEntry *)mainMenu);
  }

  void cancelFileDialogCancel()
  {
    menuUIPage.showMenu((UIMenuEntry *)mainMenu);
  }

  void goToSDFileDirectoryUIPage(void)
  {  
    switch(SD_state)
    {
      case SD_state_Unmounted:
      case SD_state_Mounted:
      case SD_state_Error:
        switch (sys.state)
        {
          case STATE_CYCLE:
          case STATE_HOMING:
            UIShowError(PSTR("  Program Running.  "));
            return;

          case STATE_HOLD:
            UIShowError(PSTR("  Feed Hold Active. "));
            return;

          case STATE_ALARM:
            UIShowError(PSTR("    Alarm Active.   "));
            return;

          case STATE_SAFETY_DOOR:
            UIShowError(PSTR("     Door Open.     "));
            return;
  
          case STATE_SLEEP:
            UIShowError(PSTR("     Sleep Mode.    "));
            return;
          
          default:
            SD_mount_card();
            if(SD_state != SD_state_Error)
              goToUIPage(&fileDirectoryUIPage);
            else
              UIShowError(PSTR("Can't open SD Card. "));
        }
      break;
    
      case SD_state_Open:
        UIShowDialogPage(PSTR("    Cancel File?    "), cancelFileDialogOK, cancelFileDialogCancel);
      break;
    }
  }

  void goToFacingOperationUIPage(void)
  {
    formUIPage.clearAllFormValues();
    formUIPage.showForm(faceOperation, faceOperationForm, PSTR("Face:             "));
  }

  void goToSlotingOperationUIPage(void)
  {
    formUIPage.clearAllFormValues();
    formUIPage.showForm(slotOperation, slotOperationForm, PSTR("Slot:             "));
  }

  void goToDrillingOperationUIPage(void)
  {
    formUIPage.clearAllFormValues();
    formUIPage.showForm(drillSingleHoleOperation, drillSingleHoleOperationForm, PSTR("Drill Single Hole:"));
  }

  void goToDrillPatternOperationUIPage(void)
  {
    formUIPage.clearAllFormValues();
    formUIPage.showForm(boltHoleCircleOperation, boltHoleCircleOperationForm, PSTR("Bolt Hole Circle: "));
  }
  
  void goToSystemMenu(void)
  {
    menuUIPage.showMenu((UIMenuEntry *)systemMenu);
  }
  
  void goToRPNCalculatorUIPage(void)
  {
		goToUIPage(&rpnCalculatorUIPage);
  }

// System Menu/Dialog functions

  void homeAllAxis(void)
  {
    // TODO: check that machine is idle before starting homing cycle.

    if(!UILineBufferIsAvailable())
    {
      UIShowError(PSTR("  Busy - try again. "));
      return;
    }
  
    if(bit_isfalse(settings.flags,BITFLAG_HOMING_ENABLE))
    {
      UIShowError(PSTR(" Homing not enabled."));
      return;
    }

    if(system_check_safety_door_ajar())
    {
      UIShowError(PSTR("    Door is open.   "));
      return;
    }
  
    UILineBuffer[0] = '$';
    UILineBuffer[1] = 'H';
    UILineBuffer[2] = 0;
    UILineBufferState = UILineBufferState_ReadyForExecution;
    goToUIPage(&statusUIPage);
  }
  
  void clearAlarm(void)
  {
    if(sys.state == STATE_ALARM)
    {
      if(system_check_safety_door_ajar())
      {
        UIShowError(PSTR("    Door is open.   "));
        return;
      }
      
      sys.state = STATE_IDLE;
    }
    
    goToUIPage(&statusUIPage);
  }
  
  void softReset(void)
  {
    mc_reset();
    goToUIPage(&statusUIPage);
  }
  
  void goToToolDataUIPage(void)
  {
    if(sys.state != STATE_IDLE)
      UIShowError(PSTR("      Not Idle.     "));
    else  
      goToUIPage(&toolDataUIPage);
  }
  
  enum axisSettingsFormValues
  {
    axisSettingsForm_steps_per_mm_X         = 0,
    axisSettingsForm_steps_per_mm_Y,
    axisSettingsForm_steps_per_mm_Z,
    
    axisSettingsForm_max_rate_X             = 4,
    axisSettingsForm_max_rate_Y,
    axisSettingsForm_max_rate_Z,
    
    axisSettingsForm_acceleration_X         = 8,
    axisSettingsForm_acceleration_Y,
    axisSettingsForm_acceleration_Z,
    
    axisSettingsForm_max_travel_X           = 12,
    axisSettingsForm_max_travel_Y,
    axisSettingsForm_max_travel_Z,
    
    axisSettingsForm_pulse_microseconds     = 16,
    axisSettingsForm_step_invert_mask,
    axisSettingsForm_dir_invert_mask,
    axisSettingsForm_flags_StepperEnableInvert,
    axisSettingsForm_flags_ProbeInvert,
    axisSettingsForm_stepper_idle_lock_time
  };
  
  UIForm axisSettingsForm[] = 
  {
    { UIFormUnsignedFloatFlag,      "Stp/mm X:" },
    { UIFormUnsignedFloatFlag,      "Stp/mm Y:" },
    { UIFormUnsignedFloatFlag,      "Stp/mm Z:" },
    UIFormSeparator,
    { UIFormUnsignedFloatFlag,      "Max Rt X:" },
    { UIFormUnsignedFloatFlag,      "Max Rt Y:" },
    { UIFormUnsignedFloatFlag,      "Max Rt Z:" },
    UIFormSeparator,
    { UIFormUnsignedFloatFlag,      " Accel X:" },
    { UIFormUnsignedFloatFlag,      " Accel Y:" },
    { UIFormUnsignedFloatFlag,      " Accel Z:" },
    UIFormSeparator,
    { UIFormUnsignedFloatFlag,      "Travel X:" },
    { UIFormUnsignedFloatFlag,      "Travel Y:" },
    { UIFormUnsignedFloatFlag,      "Travel Z:" },
    UIFormSeparator,
    { UIFormUnsignedIntegerFlag,    "Pulse Wd:" },
    { UIFormUnsignedIntegerFlag,    "StInvMsk:" },
    { UIFormUnsignedIntegerFlag,    "DrInvMsk:" },
    { UIFormCheckboxFlag,           "StEnaInv:" },
    { UIFormCheckboxFlag,           "ProbeInv:" },
    { UIFormUnsignedIntegerFlag,    "StepIdle:" },
    UIFormSeparator,
    { UIFormEndFlag,                "  Save..." }
  };
  
  void saveAxisSettings(UIFormValue *values);
  void goToAxisSettingsUIPage(void)
  {
    if(sys.state != STATE_IDLE) { UIShowError(PSTR("      Not Idle.     ")); return; }
  
    formUIPage.clearAllFormValues();
    
      formUIPage.setFormValue(axisSettingsForm_steps_per_mm_X,              (UIFormValue){ .f= settings.steps_per_mm[X_AXIS]} );
      formUIPage.setFormValue(axisSettingsForm_steps_per_mm_Y,              (UIFormValue){ .f= settings.steps_per_mm[Y_AXIS]} );
      formUIPage.setFormValue(axisSettingsForm_steps_per_mm_Z,              (UIFormValue){ .f= settings.steps_per_mm[Z_AXIS]} );
    
      formUIPage.setFormValue(axisSettingsForm_max_rate_X,                  (UIFormValue){ .f= settings.max_rate[X_AXIS]} );
      formUIPage.setFormValue(axisSettingsForm_max_rate_Y,                  (UIFormValue){ .f= settings.max_rate[Y_AXIS]} );
      formUIPage.setFormValue(axisSettingsForm_max_rate_Z,                  (UIFormValue){ .f= settings.max_rate[Z_AXIS]} );

      formUIPage.setFormValue(axisSettingsForm_acceleration_X,              (UIFormValue){ .f= settings.acceleration[X_AXIS]/(60*60)} );
      formUIPage.setFormValue(axisSettingsForm_acceleration_Y,              (UIFormValue){ .f= settings.acceleration[Y_AXIS]/(60*60)} );
      formUIPage.setFormValue(axisSettingsForm_acceleration_Z,              (UIFormValue){ .f= settings.acceleration[Z_AXIS]/(60*60)} );
    
      formUIPage.setFormValue(axisSettingsForm_max_travel_X,                (UIFormValue){ .f= -(settings.max_travel[X_AXIS])} );
      formUIPage.setFormValue(axisSettingsForm_max_travel_Y,                (UIFormValue){ .f= -(settings.max_travel[Y_AXIS])} );
      formUIPage.setFormValue(axisSettingsForm_max_travel_Z,                (UIFormValue){ .f= -(settings.max_travel[Z_AXIS])} );
    
      formUIPage.setFormValue(axisSettingsForm_pulse_microseconds,          (UIFormValue){ .u= settings.pulse_microseconds} );
      formUIPage.setFormValue(axisSettingsForm_step_invert_mask,            (UIFormValue){ .u= settings.step_invert_mask} );
      formUIPage.setFormValue(axisSettingsForm_dir_invert_mask,             (UIFormValue){ .u= settings.dir_invert_mask} );
      formUIPage.setFormValue(axisSettingsForm_flags_StepperEnableInvert,   (UIFormValue){ .b= bit_istrue(settings.flags,BITFLAG_INVERT_ST_ENABLE)} );
      formUIPage.setFormValue(axisSettingsForm_flags_ProbeInvert,           (UIFormValue){ .b= bit_istrue(settings.flags,BITFLAG_INVERT_PROBE_PIN)} );
      formUIPage.setFormValue(axisSettingsForm_stepper_idle_lock_time,      (UIFormValue){ .u= settings.stepper_idle_lock_time} );
    
    formUIPage.showForm(saveAxisSettings, axisSettingsForm, PSTR("Axis Settings:    "));
  }
  
  void saveAxisSettings(UIFormValue *values)
  {
    if(sys.state != STATE_IDLE) { UIShowError(PSTR("      Not Idle.     ")); return; }
  
    settings.steps_per_mm[X_AXIS] = values[axisSettingsForm_steps_per_mm_X].f;
    settings.steps_per_mm[Y_AXIS] = values[axisSettingsForm_steps_per_mm_Y].f;
    settings.steps_per_mm[Z_AXIS] = values[axisSettingsForm_steps_per_mm_Z].f;

    settings.max_rate[X_AXIS] = values[axisSettingsForm_max_rate_X].f;
    settings.max_rate[Y_AXIS] = values[axisSettingsForm_max_rate_Y].f;
    settings.max_rate[Z_AXIS] = values[axisSettingsForm_max_rate_Z].f;

    settings.acceleration[X_AXIS] = values[axisSettingsForm_acceleration_X].f *(60*60);
    settings.acceleration[Y_AXIS] = values[axisSettingsForm_acceleration_Y].f *(60*60);
    settings.acceleration[Z_AXIS] = values[axisSettingsForm_acceleration_Z].f *(60*60);

    settings.max_travel[X_AXIS] = -(values[axisSettingsForm_max_travel_X].f);
    settings.max_travel[Y_AXIS] = -(values[axisSettingsForm_max_travel_Y].f);
    settings.max_travel[Z_AXIS] = -(values[axisSettingsForm_max_travel_Z].f);
    
    settings.pulse_microseconds = min(values[axisSettingsForm_pulse_microseconds].u, 0xFF);
    settings.step_invert_mask = values[axisSettingsForm_step_invert_mask].u & 0xFF;
    settings.dir_invert_mask = values[axisSettingsForm_dir_invert_mask].u & 0xFF;
    
    bit_setflag(settings.flags, values[axisSettingsForm_flags_StepperEnableInvert].b, BITFLAG_INVERT_ST_ENABLE);
    bit_setflag(settings.flags, values[axisSettingsForm_flags_ProbeInvert].b, BITFLAG_INVERT_PROBE_PIN);
    
    settings.stepper_idle_lock_time = min(values[axisSettingsForm_stepper_idle_lock_time].u, 0xFF);
    
    write_global_settings();
    
    goToUIPage(&statusUIPage);
  }
  
  enum systemSettingsFormValues
  {
    systemSettingsForm_junction_deviation = 0,
    systemSettingsForm_arc_tolerance,
    
    systemSettingsForm_rpm_min = 3,
    systemSettingsForm_rpm_max,
    systemSettingsForm_flags_Laser,
    
    systemSettingsForm_flags_Homing = 7,
    systemSettingsForm_homing_dir_mask,
    systemSettingsForm_homing_feed_rate,
    systemSettingsForm_homing_seek_rate,
    systemSettingsForm_homing_debounce_delay,
    systemSettingsForm_homing_pulloff,
    systemSettingsForm_flags_LimitInvert,
    systemSettingsForm_flags_SoftLimit,
    systemSettingsForm_flags_HardLimit,
    
    systemSettingsForm_status_report_mask = 17,
    systemSettingsForm_flags_ReportInches
  };
  
  UIForm systemSettingsForm[] = 
  {
    { UIFormUnsignedFloatFlag,      "Junction:" },
    { UIFormUnsignedFloatFlag,      " Arc Tol:" },
    UIFormSeparator,
    { UIFormUnsignedIntegerFlag,    " Min RPM:" },
    { UIFormUnsignedIntegerFlag,    " Max RPM:" },
    { UIFormCheckboxFlag,           "   Laser:" },
    UIFormSeparator,
    { UIFormCheckboxFlag,           "  Homing:" },
    { UIFormUnsignedIntegerFlag,    "HmDirmsk:" },
    { UIFormUnsignedFloatFlag,      " Hm Feed:" },
    { UIFormUnsignedFloatFlag,      " Hm Seek:" },
    { UIFormUnsignedIntegerFlag,    "Debounce:" },
    { UIFormUnsignedFloatFlag,      " Pulloff:" },
    { UIFormCheckboxFlag,           "LimitInv:" },
    { UIFormCheckboxFlag,           "SftLimit:" },
    { UIFormCheckboxFlag,           "HrdLimit:" },
    UIFormSeparator,
    { UIFormUnsignedIntegerFlag,    "Rprt Msk:" },
    { UIFormCheckboxFlag,           "Rpt Inch:" },
    UIFormSeparator,
    { UIFormEndFlag,                "  Save..." }
  };
  
  void saveSystemSettings(UIFormValue *values);
  void goToSystemSettingsUIPage(void)
  {
    if(sys.state != STATE_IDLE) { UIShowError(PSTR("      Not Idle.     ")); return; }
    
    formUIPage.clearAllFormValues();
    
      formUIPage.setFormValue(systemSettingsForm_junction_deviation,        (UIFormValue){ .f= settings.junction_deviation} );
      formUIPage.setFormValue(systemSettingsForm_arc_tolerance,             (UIFormValue){ .f= settings.arc_tolerance} );
      
      formUIPage.setFormValue(systemSettingsForm_rpm_min,                   (UIFormValue){ .u= (uint32_t)(settings.rpm_min)} );
      formUIPage.setFormValue(systemSettingsForm_rpm_max,                   (UIFormValue){ .u= (uint32_t)(settings.rpm_max)} );
      formUIPage.setFormValue(systemSettingsForm_flags_Laser,               (UIFormValue){ .b= bit_istrue(settings.flags,BITFLAG_LASER_MODE)} );
      
      formUIPage.setFormValue(systemSettingsForm_flags_Homing,              (UIFormValue){ .b= bit_istrue(settings.flags,BITFLAG_HOMING_ENABLE)} );
      formUIPage.setFormValue(systemSettingsForm_homing_dir_mask,           (UIFormValue){ .u= settings.homing_dir_mask} );
      formUIPage.setFormValue(systemSettingsForm_homing_feed_rate,          (UIFormValue){ .f= settings.homing_feed_rate} );
      formUIPage.setFormValue(systemSettingsForm_homing_seek_rate,          (UIFormValue){ .f= settings.homing_seek_rate} );
      formUIPage.setFormValue(systemSettingsForm_homing_debounce_delay,     (UIFormValue){ .u= settings.homing_debounce_delay} );
      formUIPage.setFormValue(systemSettingsForm_homing_pulloff,            (UIFormValue){ .f= settings.homing_pulloff} );
      formUIPage.setFormValue(systemSettingsForm_flags_LimitInvert,         (UIFormValue){ .b= bit_istrue(settings.flags,BITFLAG_INVERT_LIMIT_PINS)} );
      formUIPage.setFormValue(systemSettingsForm_flags_SoftLimit,           (UIFormValue){ .b= bit_istrue(settings.flags,BITFLAG_SOFT_LIMIT_ENABLE)} );
      formUIPage.setFormValue(systemSettingsForm_flags_HardLimit,           (UIFormValue){ .b= bit_istrue(settings.flags,BITFLAG_HARD_LIMIT_ENABLE)} );
      
      formUIPage.setFormValue(systemSettingsForm_status_report_mask,        (UIFormValue){ .u= settings.status_report_mask} );
      formUIPage.setFormValue(systemSettingsForm_flags_ReportInches,        (UIFormValue){ .b= bit_istrue(settings.flags,BITFLAG_REPORT_INCHES)} );
      
    formUIPage.showForm(saveSystemSettings, systemSettingsForm, PSTR("System Settings:  "));
  }
  
  void saveSystemSettings(UIFormValue *values)
  {
    if(sys.state != STATE_IDLE) { UIShowError(PSTR("      Not Idle.     ")); return; }
  
    settings.junction_deviation = values[systemSettingsForm_junction_deviation].f;
    settings.arc_tolerance = values[systemSettingsForm_arc_tolerance].f;
    
    settings.rpm_min = values[systemSettingsForm_rpm_min].u;
    settings.rpm_max = values[systemSettingsForm_rpm_max].u;
    bit_setflag(settings.flags, values[systemSettingsForm_flags_Laser].b, BITFLAG_LASER_MODE);
  
    bit_setflag(settings.flags, values[systemSettingsForm_flags_Homing].b, BITFLAG_HOMING_ENABLE);
    settings.homing_dir_mask = values[systemSettingsForm_homing_dir_mask].u & 0xFF;
    settings.homing_feed_rate = values[systemSettingsForm_homing_feed_rate].f;
    settings.homing_seek_rate = values[systemSettingsForm_homing_seek_rate].f;
    settings.homing_debounce_delay = min(values[systemSettingsForm_homing_debounce_delay].u, 0xFFFF);
    settings.homing_pulloff = values[systemSettingsForm_homing_pulloff].f;
    bit_setflag(settings.flags, values[systemSettingsForm_flags_LimitInvert].b, BITFLAG_INVERT_LIMIT_PINS);
    bit_setflag(settings.flags, values[systemSettingsForm_flags_SoftLimit].b, BITFLAG_SOFT_LIMIT_ENABLE);
    bit_setflag(settings.flags, values[systemSettingsForm_flags_HardLimit].b, BITFLAG_HARD_LIMIT_ENABLE);
    
    settings.status_report_mask = values[systemSettingsForm_status_report_mask].u & 0xFF;
    bit_setflag(settings.flags, values[systemSettingsForm_flags_ReportInches].b, BITFLAG_REPORT_INCHES);
  
    write_global_settings();
  
    goToUIPage(&statusUIPage);
  }

