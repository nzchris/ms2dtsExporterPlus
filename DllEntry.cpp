
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "ms2dtsExporterPlus.h"


// Global Variables
HINSTANCE hInstance;


// This function is called by Windows when the DLL is loaded, but may also
// be called many times during time critical operations like rendering.
BOOL APIENTRY DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
   static bool controlsInit = false;

   hInstance = hinstDLL;

   if (!controlsInit)
   {
      controlsInit = true;

      INITCOMMONCONTROLSEX ic;
      ic.dwSize = sizeof(ic);
      ic.dwICC  = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
      InitCommonControlsEx(&ic);
   }

   return TRUE;
}

cMsPlugIn *CreatePlugIn()
{
   return new Ms2dtsExporterPlus();
}
