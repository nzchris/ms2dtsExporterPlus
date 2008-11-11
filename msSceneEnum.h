
#ifndef _MSSCENEENUM_H_
#define _MSSCENEENUM_H_


#include "appSceneEnum.h"
#include "ms2dtsExporterPlus.h"


namespace DTS
{
   /// This class is responsible for parsing each node in the scene, and generating
   /// the %DTS shape.
   class MsSceneEnum : public AppSceneEnum
   {
   protected:
      Ms2dtsExporterPlus *mExporter;   //!< Pointer to the exporter object

   public:
      MsSceneEnum(Ms2dtsExporterPlus *exp);

      /// <PRE>
      /// ROOT
      /// |-dummy
      /// |  |-LOD markers
      /// |  |-dummy
      /// |     |-rigid meshes
      /// |     |-skeleton
      /// |
      /// |-skinned meshes
      /// |-bounds mesh
      /// </PRE>
      /// Creates the following node hierachy from the milkshape model.
      void enumScene();
   };

}; // namespace DTS

#endif // _MSSCENEENUM_H_
