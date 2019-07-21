#include "UIConversational.h"
#include "SDSupport.h"

// Face Operation
  enum faceOperationFormValues
  {
    faceOperationForm_StartX = 0,
    faceOperationForm_StartY,
    faceOperationForm_SizeX,
    faceOperationForm_SizeY,
    faceOperationForm_StepY,
    faceOperationForm_Bidirectional,
    faceOperationForm_ClearanceZ,
    faceOperationForm_StartZ,
    faceOperationForm_Depth,
    faceOperationForm_StepZ,
    faceOperationForm_RPM,
    faceOperationForm_Feed
  };
  
  UIForm faceOperationForm[] = 
  {
    { UIFormSignedFloatFlag,        " Start X:" },
    { UIFormSignedFloatFlag,        " Start Y:" },
    { UIFormSignedFloatFlag,        "  Size X:" },
    { UIFormSignedFloatFlag,        "  Size Y:" },
    { UIFormSignedFloatFlag,        "  Y Step:" },
    { UIFormCheckboxFlag,           "  Bi-Dir:" },
    { UIFormSignedFloatFlag,        " Clear Z:" },
    { UIFormSignedFloatFlag,        " Start Z:" },
    { UIFormSignedFloatFlag,        "   Depth:" },
    { UIFormSignedFloatFlag,        "  Step Z:" },
    { UIFormSignedIntegerFlag,      "     RPM:" },
    { UIFormUnsignedFloatFlag,      "    Feed:" },
    UIFormSeparator,
    { UIFormEndFlag,                "   Run..." }
  };
  
  static inline void bidirectionalFacingPass(UIFormValue *values, bool Ydirection, float endX, float endY, float Z)
  {
    gcode_file.writeGCodeWord_f('X', values[faceOperationForm_StartX].f);
    gcode_file.writeGCodeWord_f('Y', values[faceOperationForm_StartY].f);
    gcode_file.writeEOL();
    
    gcode_file.writeGCodeWord_f('Z', values[faceOperationForm_StartZ].f);
    gcode_file.writeEOL();
  
    gcode_file.writeGCodeWord_i('G', 1);
    gcode_file.writeGCodeWord_f('F', values[faceOperationForm_Feed].f);
    gcode_file.writeGCodeWord_f('Z', Z);
        
    for(float Y = values[faceOperationForm_StartY].f;;)
    {
      gcode_file.writeGCodeWord_f('X', endX);
      gcode_file.writeEOL();
      
      if(Ydirection)
        { Y += values[faceOperationForm_StepY].f; if(Y > endY) break; }
      else
        { Y -= values[faceOperationForm_StepY].f; if(Y < endY) break; }
      
      gcode_file.writeGCodeWord_f('Y', Y);
      gcode_file.writeEOL();
      
      gcode_file.writeGCodeWord_f('X', values[faceOperationForm_StartX].f);
      gcode_file.writeEOL();
      
      if(Ydirection)
        { Y += values[faceOperationForm_StepY].f; if(Y > endY) break; }
      else
        { Y -= values[faceOperationForm_StepY].f; if(Y < endY) break; }
      
      gcode_file.writeGCodeWord_f('Y', Y);
      gcode_file.writeEOL();
    }
    
    gcode_file.writeGCodeWord_i('G', 0);
    gcode_file.writeGCodeWord_f('Z', values[faceOperationForm_ClearanceZ].f);
    gcode_file.writeEOL();
  }
  
  static inline void unidirectionalFacingPass(UIFormValue *values, bool Ydirection, float endX, float endY, float Z)
  {
    for(float Y = values[faceOperationForm_StartY].f; Ydirection?(Y < endY):(Y > endY); Ydirection?(Y += values[faceOperationForm_StepY].f):(Y -= values[faceOperationForm_StepY].f))
    {
      gcode_file.writeGCodeWord_f('X', values[faceOperationForm_StartX].f);
      gcode_file.writeGCodeWord_f('Y', Y);
      gcode_file.writeEOL();
      
      gcode_file.writeGCodeWord_f('Z', values[faceOperationForm_StartZ].f);
      gcode_file.writeEOL();
      
      gcode_file.writeGCodeWord_i('G', 1);
      gcode_file.writeGCodeWord_f('F', values[faceOperationForm_Feed].f);
      gcode_file.writeGCodeWord_f('Z', Z);
      
      gcode_file.writeGCodeWord_f('X', endX);
      gcode_file.writeEOL();
      
      gcode_file.writeGCodeWord_i('G', 0);
      gcode_file.writeGCodeWord_f('Z', values[faceOperationForm_ClearanceZ].f);
      gcode_file.writeEOL();
    }
  }
  
  void faceOperation(UIFormValue *values)
  {
    SD_open_grbltemp(PSTR("Drill.  "));
    
    values[faceOperationForm_StepY].f = fabs(values[faceOperationForm_StepY].f);
    values[faceOperationForm_StepZ].f = fabs(values[faceOperationForm_StepZ].f);
    
    bool Ydirection =  values[faceOperationForm_SizeY].f > 0;
    float endX = values[faceOperationForm_StartX].f + values[faceOperationForm_SizeX].f;
    float endY = values[faceOperationForm_StartY].f + values[faceOperationForm_SizeY].f;
    
    gcode_file.writeGCodeWord_i('G', 0);
    gcode_file.writeGCodeWord_f('Z', values[faceOperationForm_ClearanceZ].f);
    gcode_file.writeEOL();
  
    int32_t RPM = values[faceOperationForm_RPM].s;
    if(RPM > 0)
    {
      gcode_file.writeGCodeWord_i('M', 3);
      gcode_file.writeGCodeWord_l('S', RPM);
    }
    else if(RPM < 0)
    {
      gcode_file.writeGCodeWord_i('M', 4);
      gcode_file.writeGCodeWord_l('S', -RPM);
    }
    gcode_file.writeEOL();
    
    if(values[faceOperationForm_Depth].f > 0.0001)
    {
      float endZ = values[faceOperationForm_StartZ].f + values[faceOperationForm_Depth].f;
      if(values[faceOperationForm_Bidirectional].b)
      {
        float Z;
        for(Z = values[faceOperationForm_StartZ].f; Z > endZ; Z -= values[faceOperationForm_StepZ].f)
          bidirectionalFacingPass(values, Ydirection, endX, endY, Z);
        if((endZ - Z) > 0.0001) bidirectionalFacingPass(values, Ydirection, endX, endY, endZ);
      }
      else
      {
        float Z;
        for(Z = values[faceOperationForm_StartZ].f; Z > endZ; Z -= values[faceOperationForm_StepZ].f)
          unidirectionalFacingPass(values, Ydirection, endX, endY, Z);
        if((endZ - Z) > 0.0001) unidirectionalFacingPass(values, Ydirection, endX, endY, endZ);
      }
    }
    else
    {
      if(values[faceOperationForm_Bidirectional].b)
      {
          bidirectionalFacingPass(values, Ydirection, endX, endY, values[faceOperationForm_StartZ].f);
      }
      else
      {
          unidirectionalFacingPass(values, Ydirection, endX, endY, values[faceOperationForm_StartZ].f);
      }
    }
    
    gcode_file.writeGCodeWord_i('M', 5);
    gcode_file.writeEOL();
  
    SD_rewind();
  
    goToUIPage(&statusUIPage);
  }

// Slot Operation
  enum slotOperationFormValues
  {
    slotOperationForm_StartX = 0,
    slotOperationForm_StartY,
    slotOperationForm_EndX,
    slotOperationForm_EndY,
    slotOperationForm_ClearanceZ,
    slotOperationForm_StartZ,
    slotOperationForm_Depth,
    slotOperationForm_StepZ,
    slotOperationForm_RPM,
    slotOperationForm_Feed
  };
  
  UIForm slotOperationForm[] = 
  {
    { UIFormSignedFloatFlag,        " Start X:" },
    { UIFormSignedFloatFlag,        " Start Y:" },
    { UIFormSignedFloatFlag,        "   End X:" },
    { UIFormSignedFloatFlag,        "   End Y:" },
    { UIFormSignedFloatFlag,        " Clear Z:" },
    { UIFormSignedFloatFlag,        " Start Z:" },
    { UIFormSignedFloatFlag,        "   Depth:" },
    { UIFormSignedFloatFlag,        "  Step Z:" },
    { UIFormSignedIntegerFlag,      "     RPM:" },
    { UIFormUnsignedFloatFlag,      "    Feed:" },
    UIFormSeparator,
    { UIFormEndFlag,                "   Run..." }
  };
  
  static inline void slotPass(UIFormValue *values, float Z)
  {
    gcode_file.writeGCodeWord_f('X', values[slotOperationForm_StartX].f);
    gcode_file.writeGCodeWord_f('Y', values[slotOperationForm_StartX].f);
    gcode_file.writeEOL();
    
    gcode_file.writeGCodeWord_f('Z', values[slotOperationForm_StartZ].f);
    gcode_file.writeEOL();
    
    gcode_file.writeGCodeWord_i('G', 1);
    gcode_file.writeGCodeWord_f('F', values[slotOperationForm_Feed].f);
    gcode_file.writeGCodeWord_f('Z', Z);
    gcode_file.writeEOL();
    
    gcode_file.writeGCodeWord_f('X', values[slotOperationForm_EndX].f);
    gcode_file.writeGCodeWord_f('Y', values[slotOperationForm_EndY].f);
    gcode_file.writeEOL();
    
    gcode_file.writeGCodeWord_i('G', 0);
    gcode_file.writeGCodeWord_f('Z', values[slotOperationForm_ClearanceZ].f);
    gcode_file.writeEOL();
  }
  
  void slotOperation(UIFormValue *values)
  {
    SD_open_grbltemp(PSTR("Slot.  "));

    values[slotOperationForm_Depth].f = fabs(values[slotOperationForm_Depth].f);
    values[slotOperationForm_StepZ].f = fabs(values[slotOperationForm_StepZ].f);
    float endZ = values[slotOperationForm_StartZ].f - values[slotOperationForm_Depth].f;
    
    gcode_file.writeGCodeWord_i('G', 0);
    gcode_file.writeGCodeWord_f('Z', values[slotOperationForm_ClearanceZ].f);
    gcode_file.writeEOL();
  
    int32_t RPM = values[slotOperationForm_RPM].s;
    if(RPM > 0)
    {
      gcode_file.writeGCodeWord_i('M', 3);
      gcode_file.writeGCodeWord_l('S', RPM);
    }
    else if(RPM < 0)
    {
      gcode_file.writeGCodeWord_i('M', 4);
      gcode_file.writeGCodeWord_l('S', -RPM);
    }
    gcode_file.writeEOL();
    
    if(values[slotOperationForm_StepZ].f > 0.0001)
    {
      float Z;
      for(Z = values[slotOperationForm_StartZ].f; Z > endZ; Z -= values[slotOperationForm_StepZ].f)
      {
        slotPass(values, Z);
      }
      if((endZ - Z) > 0.0001) slotPass(values, endZ);
    }
    else
    {
      slotPass(values, endZ);
    }
    
    gcode_file.writeGCodeWord_i('M', 5);
    gcode_file.writeEOL();
  
    SD_rewind();
  
    goToUIPage(&statusUIPage);  
  }

// Drill Single Hole Operation
  enum drillSingleHoleOperationFormValues
  {
    drillSingleHoleOperationForm_CenterX = 0,
    drillSingleHoleOperationForm_CenterY,
    drillSingleHoleOperationForm_ClearanceZ,
    drillSingleHoleOperationForm_StartZ,
    drillSingleHoleOperationForm_Depth,
    drillSingleHoleOperationForm_PeckDistance,
    drillSingleHoleOperationForm_RPM,
    drillSingleHoleOperationForm_Feed
  };

  UIForm drillSingleHoleOperationForm[] =
  {
    { UIFormSignedFloatFlag,        "Center X:" },
    { UIFormSignedFloatFlag,        "Center Y:" },
    { UIFormSignedFloatFlag,        " Clear Z:" },
    { UIFormSignedFloatFlag,        " Start Z:" },
    { UIFormSignedFloatFlag,        "   Depth:" },
    { UIFormSignedFloatFlag,        "Peck Dst:" },
    { UIFormSignedIntegerFlag,      "     RPM:" },
    { UIFormUnsignedFloatFlag,      "    Feed:" },
    UIFormSeparator,
    { UIFormEndFlag,                "   Run..." }
  };

  static inline void drillSingleHole(float X, float Y, float clearanceZ, float startZ, float endZ, float peckDistance, float feed)
  {
    gcode_file.writeGCodeWord_f('X', X);
    gcode_file.writeGCodeWord_f('Y', Y);
    gcode_file.writeEOL();
  
    gcode_file.writeGCodeWord_f('Z', startZ);
    gcode_file.writeEOL();
  
    gcode_file.writeGCodeWord_i('G', 1);
    gcode_file.writeGCodeWord_f('F', feed);

    if(peckDistance < 0.0001)
    {
      // Standard cycle.
      gcode_file.writeGCodeWord_f('Z', endZ);
      gcode_file.writeEOL();    
    }
    else
    {
      // Peck drill cycle.
      for(float Z = startZ; Z > endZ; Z -= peckDistance)
      {
        gcode_file.writeGCodeWord_f('Z', Z);
        gcode_file.writeEOL();
      
        gcode_file.writeGCodeWord_i('G', 0);
        gcode_file.writeGCodeWord_f('Z', startZ);
        gcode_file.writeEOL();
        
        gcode_file.writeGCodeWord_f('Z', Z);
        gcode_file.writeEOL();
        
        gcode_file.writeGCodeWord_i('G', 1);
      }
      
      gcode_file.writeGCodeWord_f('Z', endZ);
      gcode_file.writeEOL();
    }
  
    gcode_file.writeGCodeWord_i('G', 0);
    gcode_file.writeGCodeWord_f('Z', clearanceZ);
    gcode_file.writeEOL();
  }

  void drillSingleHoleOperation(UIFormValue *values)
  {
    SD_open_grbltemp(PSTR("Drill.  "));
  
    gcode_file.writeGCodeWord_i('G', 0);
    gcode_file.writeGCodeWord_f('Z', values[drillSingleHoleOperationForm_ClearanceZ].f);
    gcode_file.writeEOL();
  
    int32_t RPM = values[drillSingleHoleOperationForm_RPM].s;
    if(RPM > 0)
    {
      gcode_file.writeGCodeWord_i('M', 3);
      gcode_file.writeGCodeWord_l('S', RPM);
    }
    else if(RPM < 0)
    {
      gcode_file.writeGCodeWord_i('M', 4);
      gcode_file.writeGCodeWord_l('S', -RPM);
    }
    gcode_file.writeEOL();
  
    drillSingleHole(  values[drillSingleHoleOperationForm_CenterX].f,
                      values[drillSingleHoleOperationForm_CenterY].f,
                      values[drillSingleHoleOperationForm_ClearanceZ].f,
                      values[drillSingleHoleOperationForm_StartZ].f,
                      values[drillSingleHoleOperationForm_StartZ].f + values[drillSingleHoleOperationForm_Depth].f,
                      values[drillSingleHoleOperationForm_PeckDistance].f,
                      values[drillSingleHoleOperationForm_Feed].f
                    );
  
    gcode_file.writeGCodeWord_i('M', 5);
    gcode_file.writeEOL();
  
    SD_rewind();
  
    goToUIPage(&statusUIPage);
  };

// Bolt Hole Circle Operation
  enum boltHoleCircleOperationFormValues
  {
    boltHoleCircleOperationForm_CenterX = 0,
    boltHoleCircleOperationForm_CenterY,
    boltHoleCircleOperationForm_Radius,
    boltHoleCircleOperationForm_StartAngle,
    boltHoleCircleOperationForm_ArcAngle,
    boltHoleCircleOperationForm_Count,
    boltHoleCircleOperationForm_ClearanceZ,
    boltHoleCircleOperationForm_StartZ,
    boltHoleCircleOperationForm_Depth,
    boltHoleCircleOperationForm_PeckDistance,
    boltHoleCircleOperationForm_RPM,
    boltHoleCircleOperationForm_Feed
  };


  UIForm boltHoleCircleOperationForm[] =
  {
    { UIFormSignedFloatFlag,        "Center X:" },
    { UIFormSignedFloatFlag,        "Center Y:" },
    { UIFormSignedFloatFlag,        "  Radius:" },
    { UIFormSignedFloatFlag,        "Start Dg:" },
    { UIFormSignedFloatFlag,        "  Arc Dg:" },
    { UIFormUnsignedIntegerFlag,    "   Count:" },
    { UIFormSignedFloatFlag,        " Clear Z:" },
    { UIFormSignedFloatFlag,        " Start Z:" },
    { UIFormSignedFloatFlag,        "   Depth:" },
    { UIFormSignedFloatFlag,        "Peck Dst:" },
    { UIFormSignedIntegerFlag,      "     RPM:" },
    { UIFormUnsignedFloatFlag,      "    Feed:" },
    UIFormSeparator,
    { UIFormEndFlag,                "   Run..." }
  };
  
  #define TWO_PI (6.283185307179586)
  static inline float deg2Rad(float deg)
  {
    return deg/(360.0/TWO_PI);
  }
  
  typedef struct
  {
    float x;
    float y;
  } point;
  
  static inline point pointOnArc(float centerX, float centerY, float radius, float theta)
  {
    return (point){ (cos(theta)*radius)+centerX, (sin(theta)*radius)+centerY };
  };
  
  void boltHoleCircleOperation(UIFormValue *values)
  {
    SD_open_grbltemp(PSTR("BH Circ."));
    
    values[boltHoleCircleOperationForm_StartAngle].f = deg2Rad(values[boltHoleCircleOperationForm_StartAngle].f);
    values[boltHoleCircleOperationForm_ArcAngle].f = deg2Rad(values[boltHoleCircleOperationForm_ArcAngle].f);
    
    gcode_file.writeGCodeWord_i('G', 0);
    gcode_file.writeGCodeWord_f('Z', values[drillSingleHoleOperationForm_ClearanceZ].f);
    gcode_file.writeEOL();
  
    int32_t RPM = values[drillSingleHoleOperationForm_RPM].s;
    if(RPM > 0)
    {
      gcode_file.writeGCodeWord_i('M', 3);
      gcode_file.writeGCodeWord_l('S', RPM);
    }
    else if(RPM < 0)
    {
      gcode_file.writeGCodeWord_i('M', 4);
      gcode_file.writeGCodeWord_l('S', -RPM);
    }
    gcode_file.writeEOL();
    
    float angleStep = values[boltHoleCircleOperationForm_ArcAngle].f / values[boltHoleCircleOperationForm_Count].u;
    for(int16_t index = 0; index < (int16_t)(values[boltHoleCircleOperationForm_Count].u); index++)
    {
      float theta = values[boltHoleCircleOperationForm_StartAngle].f + (index * angleStep);
      point position = pointOnArc(  values[boltHoleCircleOperationForm_CenterX].f,
                                    values[boltHoleCircleOperationForm_CenterY].f,
                                    values[boltHoleCircleOperationForm_Radius].f,
                                    theta
                                  );
  
      drillSingleHole(  position.x,
                        position.y,
                        values[boltHoleCircleOperationForm_ClearanceZ].f,
                        values[boltHoleCircleOperationForm_StartZ].f,
                        values[boltHoleCircleOperationForm_StartZ].f + values[boltHoleCircleOperationForm_Depth].f,
                        values[boltHoleCircleOperationForm_PeckDistance].f,
                        values[boltHoleCircleOperationForm_Feed].f
                      );
    }
    
    gcode_file.writeGCodeWord_i('M', 5);
    gcode_file.writeEOL();
  
    SD_rewind();
  
    goToUIPage(&statusUIPage);
  }
