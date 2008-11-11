
#ifndef _MSCONFIG_H_
#define _MSCONFIG_H_

#include "appConfig.h"


namespace DTS
{
   /// Overrides the base config class to set some defaults that are better suited
   /// to milkshape.
   class MsConfig : public AppConfig
   {
   public:
      MsConfig();

      /// Enable/disable sequence exporting
      ///
      /// @param enable   True to enable sequence export, false to disable
      void SetEnableSequences(bool enable) { mEnableSequences = enable; }
   };
};

#endif // MSCONFIG_H_

