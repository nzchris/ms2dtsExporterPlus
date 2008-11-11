
#ifndef _MS2DTSEXPORTERPLUS_H_
#define _MS2DTSEXPORTERPLUS_H_


// windows
#include <windows.h>
#include <commctrl.h>

// milkshape sdk
#include "./msLib/msLib.h"
#include "./msLib/msPlugIn.h"

// others
#include <vector>
#include "MilkshapeMesh.h"
#include "MilkshapeMaterial.h"
#include "MilkshapeSequence.h"
#include "MilkshapeBone.h"
#include "resource.h"


// allow global access to the DLL instance
extern HINSTANCE hInstance;


//--------------------------------------------------------------------------
// Default property values
namespace MsDefaults
{
   // exporter options
   const bool exportAnimation    = true;
   const bool useConfig          = false;
   const bool generateDump       = true;
   const bool copyTextures       = true;
   const bool genCS              = true;
   const bool splitDsq           = false;
};


/// The milkshape to DTS exporter. This class handles the export dialog box, as
/// well as reading milkshape meshes, materials and sequences into wrapper
/// MilkshapeNode objects.
class Ms2dtsExporterPlus : public cMsPlugIn
{
   /// Read the list of meshes, and extract mesh settings
   void readMeshList();

   /// Read export options from the milkshape model comment string
   void readExportOptions();

   /// Read the list of special materials, and extract animation sequences
   void readSpecialMaterialList();

   /// Generate a .cs file with all sequences exported
   ///
   /// @param outputFilename  Name of exported DSQ file
   void Ms2dtsExporterPlus::generateCsFile(const char *outputFilename);

   /// Deletes all allocated memory, clears exported object lists and removes
   /// any automatically added objects from the milkshape model
   void exportDone();

   /// Displays a message to the user
   ///
   /// @param title             Messagebox title
   /// @param message           Messagebox text
   void alert(const char *title, const char *message) const;

   static void updateProgress(void *arg, F32 minor, F32 major, const char* message);

public:

   Ms2dtsExporterPlus();
   ~Ms2dtsExporterPlus();

   /// Create the root bone, and bounding box mesh if required.
   void createBounds();

   /// Set a detail/reflectance/bump map material by name.
   ///
   /// @param mat   MilkshapeMaterial that will use this auxilary map
   /// @param name  Name of the auxilary map material to set
   ///
   /// @return Index of the MilkshapeMaterial of specified name
   S32 setAuxMaterial(MilkshapeMaterial **mat, const char *name) const;

   /// Update all meshes, sequences and materials with any changes made in the
   /// export dialog box.
   void applyChanges();

   //--------------------------------------------------------------------------

   /// Gets the plugin type
   ///
   /// @return The type of the plugin (used by milkshape to determine importer/exporter/tool
   int GetType() ;

   /// Gets the plugin title
   ///
   /// @return A pointer to a string containing the plugin title
   const char *GetTitle() ;

   /// Displays the settings dialog box and controls the export process.
   ///
   /// @param pModel   Pointer to the current milkshape model
   ///
   /// @return 0 if exported successfully, -1 if an error occurred
   int Execute(msModel *pModel) ;

   //--------------------------------------------------------------------------

   msModel *mModel;                 //!< The milkshape model

   // exporter options
   F32 mScale;                      //!< Scale factor applied to model
   bool mUseConfig;                 //!< Search for a configuration file
   bool mExportAnim;                //!< Export animation sequences
   bool mGenDump;                   //!< Generate a dump file
   bool mCopyTextures;              //!< Copy textures to export directory
   bool mGenCS;                     //!< Generate CS file for DSQ animations
   bool mSplitDsq;                  //!< Split export into one animation per DSQ file
   bool mExportDSQ;                 //!< Export a DSQ file

   std::vector<MilkshapeMesh*> mMeshes;         //!< Meshes to be exported
   std::vector<MilkshapeSequence*> mSequences;  //!< Sequences to be exported
   std::vector<MilkshapeSequence*> mDeletedSequences; //!< Sequences to be deleted
   std::vector<MilkshapeMaterial*> mMaterials;  //!< Materials to be exported
};

#endif // MS2DTSEXPORTERPLUS_H_

