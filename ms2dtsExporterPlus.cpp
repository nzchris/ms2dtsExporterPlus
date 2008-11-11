
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "DTSUtil.h"

#include "ms2dtsExporterPlus.h"
#include "msSceneEnum.h"
#include "msConfig.h"

#include "HtmlHelp.h"

#include <algorithm>
#include <fstream>

#include "version.h"



// Private Functions
static F32 GetDlgItemFloat(HWND hDlg, int nIDDlgItem);
static bool IsChecked(HWND hDlg, int nIDDlgItem);
static BOOL CALLBACK EditValueProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);
static void UpdateVisFrameList(HWND hDlg, int numFrames, int *frames, F32 *values);
static BOOL CALLBACK EditMeshProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK EditMaterialProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);
static void UpdateTriggerList(HWND hDlg, int numTriggers, int *frames, int *states);
static BOOL CALLBACK EditSequenceProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);
static void UpdateMeshListBox(HWND hDlg);
static void UpdateMaterialListBox(HWND hDlg);
static void UpdateSequenceListBox(HWND hDlg);
static BOOL CALLBACK ExportDlgProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK ProgressDlgProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);


// Constants
static const int MaxDlgTextLength = 256;


// Private Variables
static Ms2dtsExporterPlus *theExporter;   //!< Pointer to the current exporter object
static MilkshapeMesh *editMesh;           //!< The mesh currently being edited
static MilkshapeMaterial *editMat;        //!< The material currently being edited
static MilkshapeSequence *editSeq;        //!< The sequence currently being edited
static F32 editValue[2];                  //!< The value being edited

static msMesh *boundsMesh = NULL;         //!< The temporary bounds mesh
static msBone *rootBone = NULL;           //!< The temporary root bone

//-----------------------------------------------------------------------------
/// Get a floating point value from a dialog box control
///
/// @param   hDlg         Handle to the dialog box
/// @param   nIDDlgItem   ID of the dialog control
///
/// @return The floating point value specified in the control. 0 if none is found.
static F32 GetDlgItemFloat(HWND hDlg, int nIDDlgItem)
{
   // get dialog text
   char buff[MaxDlgTextLength+1];
   ::GetDlgItemText(hDlg, nIDDlgItem, buff, MaxDlgTextLength);

   // extract floating point value
   F32 value = 0;
   sscanf(buff, "%g", &value);
   return value;
}

/// Return the checked/unchecked state of a dialog box control
///
/// @param   hDlg         Handle to the dialog box
/// @param   nIDDlgItem   ID of the dialog control
///
/// @return True if the control is checked, false otherwise
static bool IsChecked(HWND hDlg, int nIDDlgItem)
{
   return (::IsDlgButtonChecked(hDlg, nIDDlgItem) == BST_CHECKED);
}

static void ShowHelp(char *topic)
{
   char *helpfile = "ms2dtsplus.chm::/";

   char *p;
   char *path = new char[MaxDlgTextLength + strlen(helpfile) + strlen(topic) + 1];
   ::GetModuleFileName(NULL, path, MaxDlgTextLength);
   if (p = strstr(path, "ms3d.exe"))
   {
      strcpy(p, helpfile);
      strcat(p, topic);
      ::HtmlHelp(NULL, path, HH_DISPLAY_TOPIC, 0);
     delete [] path;
   }
   else
      MessageBox(NULL, "Error", "Failed to display help topic.", MB_OK);
}

/// The dialog proc for the frame/value editor
///
/// @param   hDlg      Handle to the dialog box
/// @param   msg       Message to process
/// @param   wParam    First message parameter
/// @param   lParam    Second message parameter
///
/// @return TRUE if the message was processed, FALSE otherwise
static BOOL CALLBACK EditValueProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
   switch (msg)
   {
      case WM_INITDIALOG:
         ::SetDlgItemText(hDlg, IDC_VALUE_1, DTS::avar("%g",editValue[0]));
         ::SetDlgItemText(hDlg, IDC_VALUE_2, DTS::avar("%g",editValue[1]));
         break;

      case WM_COMMAND:

         switch (LOWORD(wParam))
         {
            case IDOK:
               editValue[0] = GetDlgItemFloat(hDlg, IDC_VALUE_1);
               editValue[1] = GetDlgItemFloat(hDlg, IDC_VALUE_2);
               ::EndDialog(hDlg, IDOK);
               break;
            case IDCANCEL:
               ::EndDialog(hDlg, IDCANCEL);
               break;
         }
         break;
   }

   return FALSE;
}


/// Update the visibility frame list box
///
/// @param hDlg        Handle to the edit mesh dialog
/// @param numFrames   Number of frames in the input array
/// @param frames      Pointer to array of frame indices
/// @param values      Pointer to array of floating point values
static void UpdateVisFrameList(HWND hDlg, int numFrames, int *frames, F32 *values)
{
   LVITEM lvi;
   lvi.mask = LVIF_TEXT;

   // clear list
   HWND listwnd = ::GetDlgItem(hDlg, IDC_MESH_VIS_LIST);
   ListView_DeleteAllItems(listwnd);

   for (int i = 0; i < numFrames; i++)
   {
      lvi.iItem = i;
      lvi.iSubItem = 0;

      char buffer[MaxDlgTextLength+1];
      lvi.pszText = buffer;
      sprintf(buffer, "%d", frames[i]);
      ListView_InsertItem(listwnd, &lvi);

      sprintf(buffer, "%g", values[i]);
      ListView_SetItemText(listwnd, i, 1, buffer);
   }
}

//-----------------------------------------------------------------------------
/// The dialog proc for the mesh editor
///
/// @param   hDlg      Handle to the dialog box
/// @param   msg       Message to process
/// @param   wParam    First message parameter
/// @param   lParam    Second message parameter
///
/// @return TRUE if the message was processed, FALSE otherwise
static BOOL CALLBACK EditMeshProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
   static int numVisFrames = 0;
   static int visFrames[MilkshapeMesh::MaxVisFrames];
   static F32 visValues[MilkshapeMesh::MaxVisFrames];

   switch (msg)
   {
      case WM_INITDIALOG:
      {
         // initialise mesh parameters
         int lod,numBigFaces,maxDepth;
         bool sorted,up,down,bb,bbz;

         editMesh->getUserPropInt("lod",lod);
         editMesh->getUserPropBool("BB",bb);
         editMesh->getUserPropBool("BBZ",bbz);
         editMesh->getUserPropBool("sorted",sorted);
         editMesh->getUserPropBool("SORT::Z_LAYER_UP",up);
         editMesh->getUserPropBool("SORT::Z_LAYER_DOWN",down);
         editMesh->getUserPropInt("SORT::NUM_BIG_FACES",numBigFaces);
         editMesh->getUserPropInt("SORT::MAX_DEPTH",maxDepth);

         ::SetDlgItemText(hDlg, IDC_MESH_NAME, editMesh->getName());
         ::SetDlgItemInt(hDlg,  IDC_MESH_LOD,  lod, TRUE);
         ::CheckDlgButton(hDlg, IDC_MESH_BB,   bb?BST_CHECKED:BST_UNCHECKED);
         ::CheckDlgButton(hDlg, IDC_MESH_BBZ,  bbz?BST_CHECKED:BST_UNCHECKED);
         ::SetDlgItemInt(hDlg, IDC_MESH_NUMFACES, numBigFaces, FALSE);
         ::SetDlgItemInt(hDlg, IDC_MESH_MAXDEPTH, maxDepth, FALSE);

         ::EnableWindow(::GetDlgItem(hDlg, IDC_MESH_UP), sorted);
         ::EnableWindow(::GetDlgItem(hDlg, IDC_MESH_DOWN), sorted);
         ::EnableWindow(::GetDlgItem(hDlg, IDC_MESH_NUMFACES), sorted);
         ::EnableWindow(::GetDlgItem(hDlg, IDC_MESH_MAXDEPTH), sorted);

         if (sorted)
         {
            ::CheckDlgButton(hDlg, IDC_MESH_SORT, BST_CHECKED);
            ::CheckDlgButton(hDlg, IDC_MESH_UP, BST_UNCHECKED);
            ::CheckDlgButton(hDlg, IDC_MESH_DOWN, BST_UNCHECKED);
            if (up)
               ::CheckDlgButton(hDlg, IDC_MESH_UP, BST_CHECKED);
            else if (down)
               ::CheckDlgButton(hDlg, IDC_MESH_DOWN, BST_CHECKED);
         }

         // populate visibility channel list box
         LVCOLUMN lvc;
         lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
         lvc.cx = 60;
         lvc.fmt = LVCFMT_LEFT;

         lvc.iSubItem = 0;
         lvc.pszText = "Frame";
         ListView_InsertColumn(::GetDlgItem(hDlg, IDC_MESH_VIS_LIST), 0, &lvc);

         lvc.iSubItem = 1;
         lvc.pszText = "Alpha";
         ListView_InsertColumn(::GetDlgItem(hDlg, IDC_MESH_VIS_LIST), 1, &lvc);
         ListView_SetExtendedListViewStyle(::GetDlgItem(hDlg, IDC_MESH_VIS_LIST), LVS_EX_FULLROWSELECT);

         numVisFrames = 0;
         editMesh->getUserPropInt("numVisFrames", numVisFrames);
         for (int i = 0; i < numVisFrames; i++)
         {
            editMesh->getUserPropInt(DTS::avar("visFrame%d", i), visFrames[i]);
            editMesh->getUserPropFloat(DTS::avar("visAlpha%d", i), visValues[i]);
         }
         UpdateVisFrameList(hDlg, numVisFrames, visFrames, visValues);

         return TRUE;
      }

      case WM_COMMAND:
         switch (LOWORD(wParam))
         {
            case IDC_MESH_SORT:
            {
               bool sorted = IsChecked(hDlg,IDC_MESH_SORT);
               if (sorted)
               {
                  // can't have a sorted billboard
                  ::CheckDlgButton(hDlg,IDC_MESH_BB,BST_UNCHECKED);
                  ::CheckDlgButton(hDlg,IDC_MESH_BBZ,BST_UNCHECKED);
               }
               ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_UP),sorted);
               ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_DOWN),sorted);
               ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_NUMFACES),sorted);
               ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_MAXDEPTH),sorted);
               break;
            }

            case IDC_MESH_BB:
               if (IsChecked(hDlg,IDC_MESH_BB))
               {
                  // can't have a sorted billboard
                  ::CheckDlgButton(hDlg,IDC_MESH_BBZ,BST_UNCHECKED);
                  ::CheckDlgButton(hDlg,IDC_MESH_SORT,BST_UNCHECKED);

                  ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_UP),false);
                  ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_DOWN),false);
                  ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_NUMFACES),false);
                  ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_MAXDEPTH),false);
               }
               break;

            case IDC_MESH_BBZ:
               if (IsChecked(hDlg,IDC_MESH_BBZ))
               {
                  // can't have a sorted billboard
                  ::CheckDlgButton(hDlg,IDC_MESH_BB,BST_UNCHECKED);
                  ::CheckDlgButton(hDlg,IDC_MESH_SORT,BST_UNCHECKED);

                  ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_UP),false);
                  ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_DOWN),false);
                  ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_NUMFACES),false);
                  ::EnableWindow(::GetDlgItem(hDlg,IDC_MESH_MAXDEPTH),false);
               }
               break;

            case IDC_MESH_UP:
               if (IsChecked(hDlg,IDC_MESH_UP))
                  ::CheckDlgButton(hDlg,IDC_MESH_DOWN,BST_UNCHECKED);
               break;

            case IDC_MESH_DOWN:
               if (IsChecked(hDlg,IDC_MESH_DOWN))
                  ::CheckDlgButton(hDlg,IDC_MESH_UP,BST_UNCHECKED);
               break;

            case IDC_MESH_VIS_ADD:
               if (numVisFrames < MilkshapeMesh::MaxVisFrames)
               {
                  // add a new visibility keyframe to the end of the list
                  visFrames[numVisFrames] = 2;
                  visValues[numVisFrames] = 1.f;
                  numVisFrames++;
                  UpdateVisFrameList(hDlg, numVisFrames, visFrames, visValues);
               }
               break;

            case IDC_MESH_VIS_EDIT:
            {
               // edit the selected frame
               int sel = ListView_GetSelectionMark(::GetDlgItem(hDlg,IDC_MESH_VIS_LIST));
               if (sel >= 0)
               {
                  editValue[0] = visFrames[sel];
                  editValue[1] = visValues[sel];
                  if (::DialogBox(hInstance, MAKEINTRESOURCE(IDD_VALUE_EDIT), hDlg, EditValueProc) == IDOK)
                  {
                     visFrames[sel] = (int)editValue[0];
                     visValues[sel] = editValue[1];
                     UpdateVisFrameList(hDlg, numVisFrames, visFrames, visValues);
                  }
               }
               break;
            }

            case IDC_MESH_VIS_REMOVE:
            {
               // remove the selected visibility frame
               int sel = ListView_GetSelectionMark(::GetDlgItem(hDlg,IDC_MESH_VIS_LIST));
               for (int k = sel; k < (numVisFrames-1); k++)
               {
                  visFrames[k] = visFrames[k+1];
                  visValues[k] = visValues[k+1];
               }
               numVisFrames--;
               UpdateVisFrameList(hDlg, numVisFrames, visFrames, visValues);
               break;
            }

            case IDC_SHOWHELP:
               ShowHelp("ms2dtsplus_meshes.html");
               break;

            case IDOK:
            {
               // get new settings
               char buffer[MaxDlgTextLength+1];
               ::GetDlgItemText(hDlg,IDC_MESH_NAME,buffer,MaxDlgTextLength);
               editMesh->setName(buffer);
               editMesh->setUserPropInt("lod",::GetDlgItemInt(hDlg,IDC_MESH_LOD,NULL,TRUE));
               editMesh->setUserPropBool("BB", IsChecked(hDlg,IDC_MESH_BB));
               editMesh->setUserPropBool("BBZ", IsChecked(hDlg,IDC_MESH_BBZ));

               bool sorted = IsChecked(hDlg,IDC_MESH_SORT);
               editMesh->setUserPropBool("sorted",sorted);

               if (sorted)
               {
                  editMesh->setUserPropBool("SORT::Z_LAYER_UP", IsChecked(hDlg,IDC_MESH_UP));
                  editMesh->setUserPropBool("SORT::Z_LAYER_DOWN", IsChecked(hDlg,IDC_MESH_DOWN));
                  editMesh->setUserPropInt("SORT::NUM_BIG_FACES",::GetDlgItemInt(hDlg,IDC_MESH_NUMFACES,NULL,FALSE));
                  editMesh->setUserPropInt("SORT::MAX_DEPTH",::GetDlgItemInt(hDlg,IDC_MESH_MAXDEPTH,NULL,FALSE));
               }

               // remove old visibility keyframes
               editMesh->clearVisFrames();

               editMesh->setUserPropInt("numVisFrames", numVisFrames);
               for (int i = 0; i < numVisFrames; i++)
               {
                  editMesh->setUserPropInt(DTS::avar("visFrame%d", i), visFrames[i]);
                  editMesh->setUserPropFloat(DTS::avar("visAlpha%d", i), visValues[i]);
               }

               ::EndDialog(hDlg, IDOK);
               break;
            }
            case IDCANCEL:
               ::EndDialog(hDlg, IDCANCEL);
               break;
         }
   }

   return FALSE;
}


//-----------------------------------------------------------------------------
/// The dialog proc for the material editor
///
/// @param   hDlg      Handle to the dialog box
/// @param   msg       Message to process
/// @param   wParam    First message parameter
/// @param   lParam    Second message parameter
///
/// @return TRUE if the message was processed, FALSE otherwise
static BOOL CALLBACK EditMaterialProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
   switch (msg)
   {
      case WM_INITDIALOG:
      {
         // populate detail, bump and reflectance map combo boxes with the
         // names of the (non-special) milkshape materials
         for (int i = 0; i < theExporter->mMaterials.size(); i++)
         {
            LPARAM name = (LPARAM)theExporter->mMaterials[i]->getName();
            ::SendDlgItemMessage(hDlg, IDC_MAT_DETAILMAP, CB_ADDSTRING, 0, name);
            ::SendDlgItemMessage(hDlg, IDC_MAT_BUMPMAP, CB_ADDSTRING, 0, name);
            ::SendDlgItemMessage(hDlg, IDC_MAT_REFLECTMAP, CB_ADDSTRING, 0, name);
         }

         // initialise material parameters
			float detailScale,reflection;
         bool add,sub,self,nomip,mipzero,trans;

			editMat->getUserPropFloat("detailScale",detailScale);
         editMat->getUserPropFloat("reflection",reflection);
         editMat->getUserPropBool("Additive",add);
         editMat->getUserPropBool("Subtractive",sub);
         editMat->getUserPropBool("SelfIlluminating",self);
         editMat->getUserPropBool("NoMipMap",nomip);
         editMat->getUserPropBool("MipMapZeroBorder",mipzero);
         editMat->getUserPropBool("Translucent",trans);

         ::SetDlgItemText(hDlg,IDC_MAT_NAME,editMat->getName());
         ::SetDlgItemText(hDlg,IDC_MAT_DETAILMAP,editMat->mDetail ? editMat->mDetail->getName() : "");
         ::SetDlgItemText(hDlg,IDC_MAT_BUMPMAP,editMat->mBump ? editMat->mBump->getName() : "");
         ::SetDlgItemText(hDlg,IDC_MAT_REFLECTMAP,editMat->mReflectance ? editMat->mReflectance->getName() : "");
         ::SetDlgItemText(hDlg,IDC_MAT_DETAIL_SCALE,DTS::avar("%g",detailScale));
         ::SetDlgItemText(hDlg,IDC_MAT_ENVMAP,DTS::avar("%g",reflection));
         ::CheckDlgButton(hDlg,IDC_MAT_ADD,add ? BST_CHECKED:BST_UNCHECKED);
         ::CheckDlgButton(hDlg,IDC_MAT_SUB,sub ? BST_CHECKED:BST_UNCHECKED);
         ::CheckDlgButton(hDlg,IDC_MAT_SELF,self ? BST_CHECKED:BST_UNCHECKED);
         ::CheckDlgButton(hDlg,IDC_MAT_NOMIPMAP,nomip ? BST_CHECKED:BST_UNCHECKED);
         ::CheckDlgButton(hDlg,IDC_MAT_MIPZERO,mipzero ? BST_CHECKED:BST_UNCHECKED);
         ::CheckDlgButton(hDlg,IDC_MAT_TRANSLUCENT,trans ? BST_CHECKED:BST_UNCHECKED);

         if (!trans)
         {
            ::EnableWindow(::GetDlgItem(hDlg,IDC_MAT_ADD), FALSE);
            ::EnableWindow(::GetDlgItem(hDlg,IDC_MAT_SUB), FALSE);
         }

         return TRUE;
      }

      case WM_COMMAND:
         switch (LOWORD(wParam))
         {
            case IDC_MAT_TRANSLUCENT:
            {
               bool trans = IsChecked(hDlg,IDC_MAT_TRANSLUCENT);
               // additive/subtractive are only valid for transparent materials
               ::EnableWindow(::GetDlgItem(hDlg,IDC_MAT_ADD), trans);
               ::EnableWindow(::GetDlgItem(hDlg,IDC_MAT_SUB), trans);
               break;
            }

            case IDC_MAT_ADD:
               // can't have both additive AND subtractive material
               if (IsChecked(hDlg,IDC_MAT_ADD))
                  ::CheckDlgButton(hDlg,IDC_MAT_SUB,BST_UNCHECKED);
               break;

            case IDC_MAT_SUB:
               // can't have both additive AND subtractive material
               if (IsChecked(hDlg,IDC_MAT_SUB))
                  ::CheckDlgButton(hDlg,IDC_MAT_ADD,BST_UNCHECKED);
               break;

            case IDC_SHOWHELP:
               ShowHelp("ms2dtsplus_materials.html");
               break;

            case IDOK:
            {
               // get new settings
               char buff[MaxDlgTextLength+1];
               ::GetDlgItemText(hDlg,IDC_MAT_NAME,buff,MaxDlgTextLength);
               editMat->setName(buff);

               ::GetDlgItemText(hDlg,IDC_MAT_DETAILMAP,buff,MaxDlgTextLength);
					S32 idx = theExporter->setAuxMaterial(&editMat->mDetail, buff);
					editMat->setUserPropInt("detail", idx);

               ::GetDlgItemText(hDlg,IDC_MAT_BUMPMAP,buff,MaxDlgTextLength);
					idx = theExporter->setAuxMaterial(&editMat->mBump, buff);
					editMat->setUserPropInt("bump", idx);

               ::GetDlgItemText(hDlg,IDC_MAT_REFLECTMAP,buff,MaxDlgTextLength);
					idx = theExporter->setAuxMaterial(&editMat->mReflectance, buff);
					editMat->setUserPropInt("reflectance", idx);

               float value = GetDlgItemFloat(hDlg, IDC_MAT_ENVMAP);
               editMat->setUserPropFloat("reflection",value);
					value = GetDlgItemFloat(hDlg, IDC_MAT_DETAIL_SCALE);
					editMat->setUserPropFloat("detailScale",value);
               editMat->setUserPropBool("NeverEnvMap", value == 0);
               editMat->setUserPropBool("Additive", IsChecked(hDlg,IDC_MAT_ADD));
               editMat->setUserPropBool("Subtractive", IsChecked(hDlg,IDC_MAT_SUB));
               editMat->setUserPropBool("SelfIlluminating", IsChecked(hDlg,IDC_MAT_SELF));
               editMat->setUserPropBool("NoMipMap", IsChecked(hDlg,IDC_MAT_NOMIPMAP));
               editMat->setUserPropBool("MipMapZeroBorder", IsChecked(hDlg,IDC_MAT_MIPZERO));
               editMat->setUserPropBool("Translucent", IsChecked(hDlg,IDC_MAT_TRANSLUCENT));
               ::EndDialog(hDlg, IDOK);
               break;
            }
            case IDCANCEL:
               ::EndDialog(hDlg, IDCANCEL);
               break;
         }
   }

   return FALSE;
}

/// Update the trigger list box
///
/// @param hDlg          Handle to the edit sequence dialog
/// @param numTriggers   Number of triggers in the input array
/// @param frames        Pointer to array of frame indices
/// @param states        Pointer to array of trigger states
static void UpdateTriggerList(HWND hDlg, int numTriggers, int *frames, int *states)
{
   LVITEM lvi;
   lvi.mask = LVIF_TEXT;

   // clear list
   ListView_DeleteAllItems(::GetDlgItem(hDlg, IDC_SEQ_TRIGGER_LIST));

   for (int i = 0; i < numTriggers; i++)
   {
      lvi.iItem = i;
      lvi.iSubItem = 0;
      lvi.pszText = (char*)DTS::avar("%i", i);
      ListView_InsertItem(::GetDlgItem(hDlg, IDC_SEQ_TRIGGER_LIST), &lvi);

      char buffer[MaxDlgTextLength+1];
      sprintf(buffer, "%d", frames[i]);
      ListView_SetItemText(::GetDlgItem(hDlg, IDC_SEQ_TRIGGER_LIST), i, 1, buffer);

      sprintf(buffer, "%d", states[i]);
      ListView_SetItemText(::GetDlgItem(hDlg, IDC_SEQ_TRIGGER_LIST), i, 2, buffer);
   }
}

//-----------------------------------------------------------------------------
/// The dialog proc for the sequence editor
///
/// @param   hDlg      Handle to the dialog box
/// @param   msg       Message to process
/// @param   wParam    First message parameter
/// @param   lParam    Second message parameter
///
/// @return TRUE if the message was processed, FALSE otherwise
static BOOL CALLBACK EditSequenceProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
   static int numTriggers = 0;
   static int triggerFrames[MilkshapeSequence::MaxTriggers];
   static int triggerStates[MilkshapeSequence::MaxTriggers];

   switch (msg)
   {
      case WM_INITDIALOG:
      {
         // initialise sequence parameters
         int startFrame,endFrame,blendRef;
         float frameRate,priority,overrideDuration;
         bool cyclic, blend, ignoreGround;
         bool enableMorph, enableTVert, enableVis, enableTransform, enableIFL;

         editSeq->getUserPropInt("startFrame",startFrame);
         editSeq->getUserPropInt("endFrame",endFrame);
         editSeq->getUserPropInt("blendReferenceFrame",blendRef);
         editSeq->getUserPropFloat("frameRate",frameRate);
         editSeq->getUserPropFloat("priority",priority);
         editSeq->getUserPropFloat("overrideDuration",overrideDuration);
         editSeq->getUserPropBool("cyclic",cyclic);
         editSeq->getUserPropBool("blend",blend);
         editSeq->getUserPropBool("ignoreGround",ignoreGround);
         editSeq->getUserPropBool("enableMorph",enableMorph);
         editSeq->getUserPropBool("enableTVert",enableTVert);
         editSeq->getUserPropBool("enableVis",enableVis);
         editSeq->getUserPropBool("enableTransform",enableTransform);
         editSeq->getUserPropBool("enableIFL",enableIFL);

         ::SetDlgItemText(hDlg,IDC_SEQ_NAME,editSeq->getName());
         ::SetDlgItemInt(hDlg,IDC_SEQ_FRAMERATE,(int)frameRate,FALSE);
         ::SetDlgItemInt(hDlg,IDC_SEQ_PRIORITY,(int)priority,FALSE);
         ::SetDlgItemInt(hDlg,IDC_SEQ_DURATION,(int)overrideDuration,TRUE);
         ::SetDlgItemInt(hDlg,IDC_SEQ_STARTFRAME,startFrame,FALSE);
         ::SetDlgItemInt(hDlg,IDC_SEQ_ENDFRAME,endFrame,FALSE);
         ::CheckDlgButton(hDlg,IDC_SEQ_CYCLIC, cyclic ? BST_CHECKED : BST_UNCHECKED);
         if (blend)
         {
            ::CheckDlgButton(hDlg,IDC_SEQ_BLEND, BST_CHECKED);
            ::EnableWindow(::GetDlgItem(hDlg,IDC_SEQ_BLEND_REF),true);
            ::SetDlgItemInt(hDlg,IDC_SEQ_BLEND_REF,blendRef,FALSE);
         }
         else
         {
            ::CheckDlgButton(hDlg,IDC_SEQ_BLEND, BST_UNCHECKED);
            ::EnableWindow(::GetDlgItem(hDlg,IDC_SEQ_BLEND_REF),false);
         }
         ::CheckDlgButton(hDlg,IDC_SEQ_IGNOREGND,  ignoreGround ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg,IDC_SEQ_EN_MORPH,   enableMorph ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg,IDC_SEQ_EN_TVERT,   enableTVert ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg,IDC_SEQ_EN_VIS,     enableVis ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg,IDC_SEQ_EN_TRANS,   enableTransform ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg,IDC_SEQ_EN_IFL,     enableIFL ? BST_CHECKED : BST_UNCHECKED);

         // trigger list columns
         struct {
            char* name;
            int width;
         } columns[] = {
             { "Trigger",  60 },
             { "Frame",   50 },
             { "State",   50 },
         };

         LVCOLUMN lvc;
         lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
         lvc.fmt = LVCFMT_CENTER;
         for (int ic = 0; ic < 3; ic++)
         {
            lvc.iSubItem = ic;
            lvc.pszText = columns[ic].name;
            lvc.cx = columns[ic].width;
            ListView_InsertColumn(::GetDlgItem(hDlg, IDC_SEQ_TRIGGER_LIST), ic, &lvc);
         }

         editSeq->getUserPropInt("numTriggers", numTriggers);
         for (int i = 0; i < numTriggers; i++)
         {
            editSeq->getUserPropInt(DTS::avar("triggerFrame%i",i), triggerFrames[i]);
            editSeq->getUserPropInt(DTS::avar("triggerState%i",i), triggerStates[i]);
         }
         UpdateTriggerList(hDlg, numTriggers, triggerFrames, triggerStates);

         ListView_SetExtendedListViewStyle(::GetDlgItem(hDlg, IDC_SEQ_TRIGGER_LIST), LVS_EX_FULLROWSELECT);
         return TRUE;
      }

      case WM_COMMAND:
         switch (LOWORD(wParam))
         {
            case IDC_SEQ_BLEND:
               if (IsChecked(hDlg,IDC_SEQ_BLEND))
                  ::EnableWindow(::GetDlgItem(hDlg,IDC_SEQ_BLEND_REF),true);
               else
                  ::EnableWindow(::GetDlgItem(hDlg,IDC_SEQ_BLEND_REF),false);
               break;

            case IDC_SEQ_TRIGGER_ADD:
               // add a new trigger to the end of the list
               if (numTriggers < MilkshapeSequence::MaxTriggers)
               {
                  triggerFrames[numTriggers] = 2;
                  triggerStates[numTriggers] = 1;
                  numTriggers++;
                  UpdateTriggerList(hDlg, numTriggers, triggerFrames, triggerStates);
               }
               break;

            case IDC_SEQ_TRIGGER_EDIT:
            {
               // edit the selected trigger
               int sel = ListView_GetSelectionMark(::GetDlgItem(hDlg,IDC_SEQ_TRIGGER_LIST));
               if (sel >= 0)
               {
                  editValue[0] = triggerFrames[sel];
                  editValue[1] = triggerStates[sel];
                  if (::DialogBox(hInstance, MAKEINTRESOURCE(IDD_VALUE_EDIT), hDlg, EditValueProc) == IDOK)
                  {
                     triggerFrames[sel] = editValue[0];
                     triggerStates[sel] = editValue[1];
                     if (triggerStates[sel] == 0)
                     {
                        ::MessageBox(hDlg, "Invalid Trigger State",
                           "Trigger states must be non-zero!", MB_APPLMODAL | MB_ICONERROR | MB_OK);
                        triggerStates[sel] = 1;
                     }
                     UpdateTriggerList(hDlg, numTriggers, triggerFrames, triggerStates);
                  }
               }
               break;
            }

            case IDC_SEQ_TRIGGER_REMOVE:
            {
               // remove the selected trigger
               int sel = ListView_GetSelectionMark(::GetDlgItem(hDlg,IDC_SEQ_TRIGGER_LIST));
               for (int k = sel; k < (numTriggers-1); k++)
               {
                  triggerFrames[k] = triggerFrames[k+1];
                  triggerStates[k] = triggerStates[k+1];
               }
               numTriggers--;
               UpdateTriggerList(hDlg, numTriggers, triggerFrames, triggerStates);
               break;
            }

            case IDC_SHOWHELP:
               ShowHelp("ms2dtsplus_animation.html");
               break;

            case IDOK:
            {
               // get new settings
               char buffer[MaxDlgTextLength+1];
               ::GetDlgItemText(hDlg,IDC_SEQ_NAME,buffer,MaxDlgTextLength);

               float duration = (float)(int)::GetDlgItemInt(hDlg,IDC_SEQ_DURATION,NULL,TRUE);

               editSeq->setName(buffer);
               editSeq->setUserPropFloat("frameRate",(float)::GetDlgItemInt(hDlg,IDC_SEQ_FRAMERATE,NULL,FALSE));
               editSeq->setUserPropFloat("priority",(float)::GetDlgItemInt(hDlg,IDC_SEQ_PRIORITY,NULL,FALSE));
               editSeq->setUserPropFloat("overrideDuration",(float)(int)::GetDlgItemInt(hDlg,IDC_SEQ_DURATION,NULL,TRUE));
               editSeq->setUserPropInt("startFrame",::GetDlgItemInt(hDlg,IDC_SEQ_STARTFRAME,NULL,FALSE));
               editSeq->setUserPropInt("endFrame",::GetDlgItemInt(hDlg,IDC_SEQ_ENDFRAME,NULL,FALSE));
               editSeq->setUserPropInt("blendReferenceFrame",::GetDlgItemInt(hDlg,IDC_SEQ_BLEND_REF,NULL,FALSE));

               editSeq->setUserPropBool("cyclic", IsChecked(hDlg,IDC_SEQ_CYCLIC));
               editSeq->setUserPropBool("blend", IsChecked(hDlg,IDC_SEQ_BLEND));
               editSeq->setUserPropBool("ignoreGround", IsChecked(hDlg,IDC_SEQ_IGNOREGND));
               editSeq->setUserPropBool("enableMorph", IsChecked(hDlg,IDC_SEQ_EN_MORPH));
               editSeq->setUserPropBool("enableTVert", IsChecked(hDlg,IDC_SEQ_EN_TVERT));
               editSeq->setUserPropBool("enableVis", IsChecked(hDlg,IDC_SEQ_EN_VIS));
               editSeq->setUserPropBool("enableTransform", IsChecked(hDlg,IDC_SEQ_EN_TRANS));
               editSeq->setUserPropBool("enableIFL", IsChecked(hDlg,IDC_SEQ_EN_IFL));

               // remove old trigger keyframes
               editSeq->clearTriggers();

               // set triggers
               editSeq->setUserPropInt("numTriggers", numTriggers);
               for (int i = 0; i < numTriggers; i++)
               {
                  editSeq->setUserPropInt(DTS::avar("triggerFrame%i",i), triggerFrames[i]);
                  editSeq->setUserPropInt(DTS::avar("triggerState%i",i), triggerStates[i]);
               }

               ::EndDialog(hDlg, IDOK);
               break;
            }
            case IDCANCEL:
               ::EndDialog(hDlg, IDCANCEL);
               break;
         }
   }

   return FALSE;
}


//-----------------------------------------------------------------------------
/// Updates the mesh list box
///
/// @param hDlg   Handle to the exporter dialog box
static void UpdateMeshListBox(HWND hDlg)
{
   LVITEM lvi;
   lvi.mask = LVIF_TEXT;

   // clear list
   HWND listwnd = ::GetDlgItem(hDlg, IDC_MESH_LIST);
   ListView_DeleteAllItems(listwnd);

   for (int i = 0; i < theExporter->mMeshes.size(); i++)
   {
      // sequence list items
      MilkshapeMesh *mesh = theExporter->mMeshes[i];

      lvi.iItem = i;
      lvi.iSubItem = 0;
      lvi.pszText = const_cast<char*>(mesh->getName());
      ListView_InsertItem(listwnd, &lvi);

      int lod;
      char buffer[MaxDlgTextLength+1];
      mesh->getUserPropInt("lod",lod);
      sprintf(buffer,"%d",lod);
      ListView_SetItemText(listwnd, i, 1, buffer);
   }
}

/// Updates the material list box
///
/// @param hDlg   Handle to the exporter dialog box
static void UpdateMaterialListBox(HWND hDlg)
{
   LVITEM lvi;
   lvi.mask = LVIF_TEXT;

   // clear list
   HWND listwnd = ::GetDlgItem(hDlg, IDC_MAT_LIST);
   ListView_DeleteAllItems(listwnd);

   int count = 0;
   for (int i = 0; i < theExporter->mMaterials.size(); i++)
   {
      // sequence list items
      MilkshapeMaterial *mat = theExporter->mMaterials[i];

      // only display materials that are to be exported (either attached to a
      // mesh or used as an auxilary map)
		if (mat->mRefCount)
		{
			lvi.iItem = count++;
         lvi.iSubItem = 0;
         lvi.pszText = const_cast<char*>(mat->getName());
         ListView_InsertItem(listwnd, &lvi);
		}
   }
}

/// Updates the sequence list box
///
/// @param hDlg   Handle to the exporter dialog box
static void UpdateSequenceListBox(HWND hDlg)
{
   LVITEM lvi;
   lvi.mask = LVIF_TEXT;

   // clear list
   HWND listwnd = ::GetDlgItem(hDlg, IDC_SEQUENCE_LIST);
   ListView_DeleteAllItems(listwnd);

   for (int i = 0; i < theExporter->mSequences.size(); i++)
   {
      // sequence list items
      MilkshapeSequence *seq = theExporter->mSequences[i];
      int startFrame,endFrame;
      bool cyclic;

      seq->getUserPropInt("startFrame",startFrame);
      seq->getUserPropInt("endFrame",endFrame);
      seq->getUserPropBool("cyclic",cyclic);

      lvi.iItem = i;
      lvi.iSubItem = 0;
      lvi.pszText = const_cast<char*>(seq->getName());
      ListView_InsertItem(listwnd, &lvi);

      char buffer[MaxDlgTextLength+1];
      sprintf(buffer,"%d",startFrame);
      ListView_SetItemText(listwnd, i, 1, buffer);

      sprintf(buffer,"%d",endFrame);
      ListView_SetItemText(listwnd, i, 2, buffer);

      sprintf(buffer,"%s",cyclic?"Yes":"No");
      ListView_SetItemText(listwnd, i, 3, buffer);
   }
}

/// The dialog proc for the export dialog box
///
/// @param   hDlg      Handle to the dialog box
/// @param   msg       Message to process
/// @param   wParam    First message parameter
/// @param   lParam    Second message parameter
///
/// @return TRUE if the message was processed, FALSE otherwise
static BOOL CALLBACK ExportDlgProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
   int sel;

   switch (msg)
   {
      case WM_INITDIALOG:
      {
         // set window title
         ::SetWindowText(hDlg, "Torque DTSPlus Exporter " VERSION_NUMBER_STR);

         // populate mesh, material and sequence list boxes
         ListView_SetExtendedListViewStyle(::GetDlgItem(hDlg, IDC_MESH_LIST), LVS_EX_FULLROWSELECT);
         ListView_SetExtendedListViewStyle(::GetDlgItem(hDlg, IDC_MAT_LIST), LVS_EX_FULLROWSELECT);
         ListView_SetExtendedListViewStyle(::GetDlgItem(hDlg, IDC_SEQUENCE_LIST), LVS_EX_FULLROWSELECT);

         struct {
            char* name;
            int width;
         } columns[] = {
             { "Name",  100 },
             { "LOD",   40 },
             { "End",   40 },
             { "Cyclic",40 },
         };

         LVCOLUMN lvc;
         lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
         for (int ic = 0; ic < 2; ic++)
         {
            // mesh list columns
            lvc.iSubItem = ic;
            lvc.pszText = columns[ic].name;
            lvc.cx = columns[ic].width;
            lvc.fmt = (!ic)? LVCFMT_LEFT: LVCFMT_CENTER;
            ListView_InsertColumn(::GetDlgItem(hDlg, IDC_MESH_LIST), ic, &lvc);
         }
         for (ic = 0; ic < 1; ic++)
         {
            // material list columns
            lvc.iSubItem = ic;
            lvc.pszText = columns[ic].name;
            lvc.cx = columns[ic].width;
            lvc.fmt = (!ic)? LVCFMT_LEFT: LVCFMT_CENTER;
            ListView_InsertColumn(::GetDlgItem(hDlg, IDC_MAT_LIST), ic, &lvc);
         }
         columns[1].name = "Start";
         for (ic = 0; ic < 4; ic++)
         {
            // sequence list columns
            lvc.iSubItem = ic;
            lvc.pszText = columns[ic].name;
            lvc.cx = columns[ic].width;
            lvc.fmt = (!ic)? LVCFMT_LEFT: LVCFMT_CENTER;
            ListView_InsertColumn(::GetDlgItem(hDlg, IDC_SEQUENCE_LIST), ic, &lvc);
         }

         UpdateMeshListBox(hDlg);
         UpdateMaterialListBox(hDlg);
         UpdateSequenceListBox(hDlg);

         // set options
         ::SetDlgItemText(hDlg, IDC_OPT_SCALE, DTS::avar("%g",theExporter->mScale));
         ::CheckDlgButton(hDlg, IDC_EXPORT_DSQ, theExporter->mExportDSQ ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg, IDC_EXPORT_ANIM, theExporter->mExportAnim ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg, IDC_USE_CONFIG, theExporter->mUseConfig ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg, IDC_OUTPUT_DUMP, theExporter->mGenDump ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg, IDC_COPY_TEXTURES, theExporter->mCopyTextures ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg, IDC_GEN_CS_FILE, theExporter->mGenCS ? BST_CHECKED : BST_UNCHECKED);
         ::CheckDlgButton(hDlg, IDC_SPLIT_DSQ, theExporter->mSplitDsq ? BST_CHECKED : BST_UNCHECKED);

         return TRUE;
      }

      case WM_COMMAND:
      {
         switch (LOWORD(wParam))
         {
            case IDC_MESH_EDIT:
               // edit the selected mesh
               sel = ListView_GetSelectionMark(::GetDlgItem(hDlg,IDC_MESH_LIST));
               if (sel < 0)
                  break;
               editMesh = theExporter->mMeshes[sel];
               if (::DialogBox(hInstance, MAKEINTRESOURCE(IDD_MESH_EDIT), hDlg, EditMeshProc) == IDOK)
               {
                  // apply new settings
                  UpdateMeshListBox(hDlg);
               }
               break;

            case IDC_MAT_EDIT:
               // edit the selected material
               sel = ListView_GetSelectionMark(::GetDlgItem(hDlg,IDC_MAT_LIST));
               if (sel < 0)
                  break;
               editMat = theExporter->mMaterials[sel];
               if (::DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAT_EDIT), hDlg, EditMaterialProc) == IDOK)
               {
                  // apply new settings
                  UpdateMaterialListBox(hDlg);
               }
               break;

            case IDC_SEQ_ADD:
            {
               MilkshapeSequence *newSeq = new MilkshapeSequence();
               editSeq = newSeq;

               if (::DialogBox(hInstance, MAKEINTRESOURCE(IDD_SEQUENCE_EDIT), hDlg, EditSequenceProc) == IDOK)
               {
                  // add a new sequence to the end of the list
                  theExporter->mSequences.push_back(newSeq);
                  UpdateSequenceListBox(hDlg);
               }
               else
                  delete newSeq;
               break;
            }

            case IDC_SEQ_EDIT:
               // edit the selected sequence
               sel = ListView_GetSelectionMark(::GetDlgItem(hDlg,IDC_SEQUENCE_LIST));
               if (sel < 0)
                  break;
               editSeq = theExporter->mSequences[sel];
               if (::DialogBox(hInstance, MAKEINTRESOURCE(IDD_SEQUENCE_EDIT), hDlg, EditSequenceProc) == IDOK)
               {
                  // apply new settings
                  UpdateSequenceListBox(hDlg);
               }
               break;

            case IDC_SEQ_REMOVE:
               // remove the selected sequence from the list
               sel = ListView_GetSelectionMark(::GetDlgItem(hDlg,IDC_SEQUENCE_LIST));
               if (sel >= 0)
               {
                  // add this sequence to the 'to be deleted' list
                  theExporter->mDeletedSequences.push_back(theExporter->mSequences[sel]);
                  theExporter->mSequences.erase(theExporter->mSequences.begin() + sel);
                  UpdateSequenceListBox(hDlg);
               }
               break;

            case IDC_CREATE_BOUNDS:
               theExporter->createBounds();
               break;

            case IDC_SHOWHELP:
               ShowHelp("ms2dtsplus_main.html");
               break;

            case IDCANCEL:
               // quit without applying changes
               ::EndDialog(hDlg, IDCANCEL);
               break;

            case IDC_APPLY:
            case IDC_EXPORT:
            case IDC_EXPORT_DSQ:
            {
               // get options from dialog box settings
               theExporter->mExportDSQ = (LOWORD(wParam) == IDC_EXPORT_DSQ);
               theExporter->mScale = GetDlgItemFloat(hDlg, IDC_OPT_SCALE);
               theExporter->mExportAnim = IsChecked(hDlg, IDC_EXPORT_ANIM);
               theExporter->mUseConfig = IsChecked(hDlg, IDC_USE_CONFIG);
               theExporter->mGenDump = IsChecked(hDlg, IDC_OUTPUT_DUMP);
               theExporter->mCopyTextures = IsChecked(hDlg, IDC_COPY_TEXTURES);
               theExporter->mGenCS = IsChecked(hDlg, IDC_GEN_CS_FILE);
               theExporter->mSplitDsq = IsChecked(hDlg, IDC_SPLIT_DSQ);

               theExporter->applyChanges();
               if (LOWORD(wParam) == IDC_APPLY)
               {
                  // if the user has explicitly created the bounds mesh, we
                  // don't want to delete it after exporting, so clear the root
                  // bone and bounds mesh pointers
                  rootBone = NULL;
                  boundsMesh = NULL;
                  break;
               }

               ::EndDialog(hDlg, IDOK);
               break;
            }
         }
      }
   }

   return FALSE;
}

static BOOL CALLBACK ProgressDlgProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
   switch (msg)
   {
   case WM_INITDIALOG:
      // set window title
      ::SetWindowText(hDlg, "Exporting shape - please wait...");

      // initialise progress bar and text
      ::SendDlgItemMessage(hDlg, IDC_PROGRESS_BAR, PBM_SETPOS, 0, 0);
      ::SendDlgItemMessage(hDlg, IDC_PROGRESS_BAR, PBM_SETRANGE, 0, MAKELPARAM(0, 70));
      ::SendDlgItemMessage(hDlg, IDC_PROGRESS_BAR, PBM_SETSTEP, 10, 0);
      ::SetDlgItemText(hDlg, IDC_PROGRESS_TEXT, "");
      return TRUE;
   }

   return FALSE;
}

//-----------------------------------------------------------------------------
Ms2dtsExporterPlus::Ms2dtsExporterPlus()
{
   // set default options
   mExportDSQ = false;
   mScale = MsDefaults::modelScale;
   mExportAnim = MsDefaults::exportAnimation;
   mUseConfig = MsDefaults::useConfig;
   mGenDump = MsDefaults::generateDump;
   mCopyTextures = MsDefaults::copyTextures;
   mGenCS = MsDefaults::genCS;
   mSplitDsq = MsDefaults::splitDsq;
}

Ms2dtsExporterPlus::~Ms2dtsExporterPlus()
{
   exportDone();
}

void Ms2dtsExporterPlus::readExportOptions()
{
   // get model comment string
   char commentString[MAX_COMMENT_LENGTH+1] = "";
   msModel_GetComment(mModel, commentString, MAX_COMMENT_LENGTH);

   // parse comment for export options
   float s;
   char c;

   // each option is stored as a "name=value" pair
   const char *delims = "\r\n";
   char *p = strtok(commentString, delims);
   while (p)
   {
      if (sscanf(p, "scale=%f", &s) == 1)
         mScale = s;
      else if (sscanf(p, "config=%c", &c) == 1)
         mUseConfig = (c == '1');
      else if (sscanf(p, "anim=%c", &c) == 1)
         mExportAnim = (c == '1');
      else if (sscanf(p, "dump=%c", &c) == 1)
         mGenDump = (c == '1');
      else if (sscanf(p, "copy=%c", &c) == 1)
         mCopyTextures = (c == '1');
      else if (sscanf(p, "gencs=%c", &c) == 1)
         mGenCS = (c == '1');
      else if (sscanf(p, "splitDsq=%c", &c) == 1)
         mSplitDsq = (c == '1');

      p = strtok(NULL, delims);
   }
}

void Ms2dtsExporterPlus::readMeshList()
{
	// initially add all materials (not special materials), but leave mRefCount = 0
	// as they may not be in use
	for (int i = 0; i < msModel_GetMaterialCount(mModel); i++)
	{
		msMaterial *msMat = msModel_GetMaterialAt(mModel, i);

      // check for special materials
      char name[MS_MAX_NAME+1];
      msMaterial_GetName(msMat, name, MS_MAX_NAME);
      if (strStartsWith(name, "*") || strStartsWith(name, "seq:"))
         continue;

		// create the material
		MilkshapeMaterial *mat = new MilkshapeMaterial(i);
		mMaterials.push_back(mat);
	}

   // read meshes
   for (i = 0; i < msModel_GetMeshCount(mModel); i++)
   {
      // create mesh
      MilkshapeMesh *meshDat = new MilkshapeMesh(i);
      mMeshes.push_back(meshDat);

      // now find the material assigned to this mesh
      int materialIndex = msMesh_GetMaterialIndex(meshDat->getMsMesh());
      if (materialIndex < 0)
         continue;

      msMaterial *msMat = msModel_GetMaterialAt(mModel, materialIndex);

      // if this meshes material has not already been added, it must start
      // with one of the special identifiers => these special materials should
      // NOT be assigned to a mesh!
      std::vector<MilkshapeMaterial*>::const_iterator it = mMaterials.begin();
      while (it != mMaterials.end())
      {
         if ((*it)->getMsMaterial() == msMat)
         {
            meshDat->setMaterial(*it);
            (*it)->mRefCount++;
            break;
         }
         it++;
      }

      if (it == mMaterials.end())
      {
         // warn the user that this mesh has an invalid material
         char invalidName[MS_MAX_NAME+1];
         msMaterial_GetName(msMat, invalidName, MS_MAX_NAME);

         alert("Invalid material name",
            DTS::avar("The material (%s) attached to mesh %s has an invalid name!"
            " Special identifiers ('*' and 'seq:') cannot be used in regular material names.",
            invalidName, meshDat->getName()) );
      }
   }

	// setup auxilary maps for each material
	for (i = 0; i < mMaterials.size(); i++)
	{
		int detail=-1, bump=-1, reflect=-1;

		mMaterials[i]->getUserPropInt("detail", detail);
		mMaterials[i]->getUserPropInt("bump", bump);
		mMaterials[i]->getUserPropInt("reflectance", reflect);

		if ((detail >= 0) && (detail < mMaterials.size()))
		{
			mMaterials[i]->mDetail = mMaterials[detail];
			mMaterials[detail]->mRefCount++;
		}
		if ((bump >= 0) && (bump < mMaterials.size()))
		{
			mMaterials[i]->mBump = mMaterials[bump];
			mMaterials[bump]->mRefCount++;
		}
		if ((reflect >= 0) && (reflect < mMaterials.size()))
		{
			mMaterials[i]->mReflectance = mMaterials[reflect];
			mMaterials[reflect]->mRefCount++;
		}
	}
}

S32 Ms2dtsExporterPlus::setAuxMaterial(MilkshapeMaterial **mat, const char *name) const
{
	MilkshapeMaterial *auxMat = NULL;
	S32 index = -1;

	// find the material that matches the given name
	for (int i = 0; i < mMaterials.size(); i++)
	{
		if (strcmp(mMaterials[i]->getName(), name) == 0)
		{
			index = i;
			auxMat = mMaterials[i];
			break;
		}
	}

	// changing material?
	if (auxMat != *mat)
	{
		if (*mat)
			(*mat)->mRefCount--;
		if (auxMat)
			auxMat->mRefCount++;
						
		*mat = auxMat;
	}

	return index;
}

void Ms2dtsExporterPlus::readSpecialMaterialList()
{
   for (int i = 0; i < msModel_GetMaterialCount(mModel); i++)
   {
      char name[MS_MAX_NAME+1];

      // get material name
      msMaterial *msMat = msModel_GetMaterialAt(mModel, i);
      msMaterial_GetName(msMat, name, MS_MAX_NAME);

      // check for special materials
      if (!strStartsWith(name, "*") && !strStartsWith(name, "seq:"))
         continue;

      // try to create an animation sequence
      MilkshapeSequence *seq = new MilkshapeSequence(i, name);
      if (!seq->getMsMaterial())
         delete seq;
      else
         mSequences.push_back(seq);
   }
}


//-----------------------------------------------------------------------------
void Ms2dtsExporterPlus::applyChanges()
{
   //--------------------------------------------------------------------------
   // store options in milkshape model comment string

   // generate comment string
   char commentString[MAX_COMMENT_LENGTH+1];
   strcpy(commentString, DTS::avar("scale=%f\r\n", mScale));
   strcat(commentString, DTS::avar("config=%d\r\n", (int)mUseConfig));
   strcat(commentString, DTS::avar("anim=%d\r\n", (int)mExportAnim));
   strcat(commentString, DTS::avar("dump=%d\r\n", (int)mGenDump));
   strcat(commentString, DTS::avar("copy=%d\r\n", (int)mCopyTextures));
   strcat(commentString, DTS::avar("gencs=%d\r\n", (int)mGenCS));
   strcat(commentString, DTS::avar("splitDsq=%d\r\n", (int)mSplitDsq));
   msModel_SetComment(mModel, commentString);
   MilkshapeNode::setScale(mScale);

   //--------------------------------------------------------------------------
   // update meshes
   for (int i = 0; i < mMeshes.size(); i++)
   {
      MilkshapeMesh *mesh = mMeshes[i];

      // set name+lod
      int lod;
      mesh->getUserPropInt("lod", lod);
      msMesh_SetName(mesh->getMsMesh(), DTS::avar("%s%d", mesh->getName(), lod));

      // store mesh information in milkshape mesh comment
      mesh->writePropertyString(commentString, MAX_COMMENT_LENGTH);
      msMesh_SetComment(mesh->getMsMesh(), commentString);
   }

   //--------------------------------------------------------------------------
   // update materials
   for (i = 0; i < mMaterials.size(); i++)
   {
      MilkshapeMaterial *mat = mMaterials[i];

      // set name
      msMaterial_SetName(mat->getMsMaterial(), mat->getName());

      // set NeverEnvMap flag based on amount of environment mapping
      float value = 0;
      mat->getUserPropFloat("reflection", value);
      mat->setUserPropBool("NeverEnvMap", value == 0);

      // store material information in milkshape material comment
      mat->writePropertyString(commentString, MAX_COMMENT_LENGTH);
      msMaterial_SetComment(mat->getMsMaterial(), commentString);
   }

   //--------------------------------------------------------------------------
   // update sequence special materials
   for (i = 0; i < mSequences.size(); i++)
   {
      MilkshapeSequence *seq = mSequences[i];

      if (!seq->getMsMaterial())
      {
         // create a new sequence
         msModel_AddMaterial(mModel);
         seq->setMatIndex(msModel_GetMaterialCount(mModel) - 1);
      }

      // set name
      msMaterial_SetName(seq->getMsMaterial(), DTS::avar("*%s", seq->getName()));

      // store sequence information in milkshape material comment
      seq->writePropertyString(commentString, MAX_COMMENT_LENGTH);
      msMaterial_SetComment(seq->getMsMaterial(), commentString);
   }

   // delete removed sequences
   for (i = 0; i < mDeletedSequences.size(); i++)
   {
      MilkshapeSequence *rem = mDeletedSequences[i];
      if (rem->getMsMaterial())
      {
         // update material indices
         for (int k = 0; k < mSequences.size(); k++)
         {
            MilkshapeSequence *seq = mSequences[k];
            if (seq->getMatIndex() > rem->getMatIndex())
               seq->setMatIndex(seq->getMatIndex() - 1);
         }
         for (k = i+1; k < mDeletedSequences.size(); k++)
         {
            MilkshapeSequence *seq = mDeletedSequences[k];
            if (seq->getMatIndex() > rem->getMatIndex())
               seq->setMatIndex(seq->getMatIndex() - 1);
         }

         // remove material from milkshape model
         msMaterial_Destroy(mModel, rem->getMsMaterial());
      }
      delete rem;
   }
   mDeletedSequences.clear();
}

void Ms2dtsExporterPlus::createBounds()
{
   //--------------------------------------------------------------------------
   // check if a root node already exists
   if ((msModel_FindBoneByName(mModel, "root") < 0) &&
      (msModel_FindBoneByName(mModel, "Root") < 0))
   {
      // add a dummy root bone to the model to catch any vertices not assigned
      // to a bone
      rootBone = msModel_GetBoneAt(mModel, msModel_AddBone(mModel));
      msVec3 pos = { 0,0,0 };
      msVec3 rot = { 0,0,0 };
      msBone_SetPosition(rootBone, pos);
      msBone_SetRotation(rootBone, rot);
      msBone_SetName(rootBone, "Root");
   }

   // check if a bounding box already exists
   int numMeshes = msModel_GetMeshCount(mModel);
   for (int i = 0; i < numMeshes; i++)
   {
      int size;
      char name[MS_MAX_NAME+1];
      msMesh *mesh = msModel_GetMeshAt(mModel, i);
      msMesh_GetName(mesh, name, MS_MAX_NAME);
      char *p = DTS::chopTrailingNumber(name, size);
      if (p && (stricmp(p, "bounds") == 0))
      {
         delete [] p;
         return;
      }
      delete [] p;
   }

   //--------------------------------------------------------------------------
   // determine bounds size by finding min/max X, Y, Z
   DTS::Point3D pMin(99999999.f, 99999999.f, 99999999.f);
   DTS::Point3D pMax(-99999999.f, -99999999.f, -99999999.f);
   for (i = 0; i < numMeshes; i++)
   {
      msMesh *mesh = msModel_GetMeshAt(mModel, i);

      int numVtx = msMesh_GetVertexCount(mesh);
      for (int k = 0; k < numVtx; k++)
      {
         msVertex *vtx = msMesh_GetVertexAt(mesh, k);
         MilkshapePoint p(vtx->Vertex);

         // set min
         if (p.x() < pMin.x()) pMin.x(p.x() - 1);
         if (p.y() < pMin.y()) pMin.y(p.y() - 1);
         if (p.z() < pMin.z()) pMin.z(p.z() - 1);
         // set max
         if (p.x() > pMax.x()) pMax.x(p.x() + 1);
         if (p.y() > pMax.y()) pMax.y(p.y() + 1);
         if (p.z() > pMax.z()) pMax.z(p.z() + 1);
      }
   }

   // create bounds mesh
   boundsMesh = msModel_GetMeshAt(mModel, msModel_AddMesh(mModel));
   if (!boundsMesh)
   {
      alert("Export error", "Could not create bounds mesh");
      return;
   }
   msMesh_SetName(boundsMesh, "Bounds");
   mMeshes.push_back(new MilkshapeMesh(msModel_GetMeshCount(mModel)-1));

   // add bounds vertices
   //    7----6
   //   /    /|
   //  /    / |  Y
   // 3----2  5
   // |    | /
   // |    |/  Z
   // 0----1
   //   X
   msVec3 vec[8];
   vec[0][0] = -pMin.x(); vec[0][1] = pMin.z(); vec[0][2] = pMin.y();
   vec[1][0] = -pMax.x(); vec[1][1] = pMin.z(); vec[1][2] = pMin.y();
   vec[2][0] = -pMax.x(); vec[2][1] = pMax.z(); vec[2][2] = pMin.y();
   vec[3][0] = -pMin.x(); vec[3][1] = pMax.z(); vec[3][2] = pMin.y();
   vec[4][0] = -pMin.x(); vec[4][1] = pMin.z(); vec[4][2] = pMax.y();
   vec[5][0] = -pMax.x(); vec[5][1] = pMin.z(); vec[5][2] = pMax.y();
   vec[6][0] = -pMax.x(); vec[6][1] = pMax.z(); vec[6][2] = pMax.y();
   vec[7][0] = -pMin.x(); vec[7][1] = pMax.z(); vec[7][2] = pMax.y();

   for (i = 0; i < 8; i++)
   {
      msMesh_AddVertex(boundsMesh);
      msVertex* vtx = msMesh_GetVertexAt(boundsMesh, i);
      msVertex_SetVertex(vtx, vec[i]);
   }

   // add bounds normals and faces
   // normals don't actually matter for this mesh, but they are needed
   // for mesh construction
   msVec3 normal = { 1.f, 0.f, 0.f };
   word indices[] = {   0, 1, 2,    0, 2, 3, \
                        1, 5, 6,    1, 6, 2, \
                        5, 4, 7,    5, 7, 6, \
                        4, 0, 3,    4, 3, 7, \
                        3, 2, 6,    3, 6, 7, \
                        0, 4, 5,    0, 5, 1     };

   for (i = 0; i < 12; i++)
   {
      msMesh_AddTriangle(boundsMesh);
      msTriangle* face = msMesh_GetTriangleAt(boundsMesh, i);
      msTriangle_SetVertexIndices(face, &indices[i*3]);
      msMesh_SetVertexNormalAt(boundsMesh, msMesh_AddVertexNormal(boundsMesh), normal);
   }
}

void Ms2dtsExporterPlus::generateCsFile(const char *outputFilename)
{
   // get name of shape (just the DSQ filename without the extension)
   char *shapeName = new char[strlen(outputFilename)];
   strcpy(shapeName, DTS::getFileBase(outputFilename));
   char *p = strrchr(shapeName, '.');
   if (p)
      *p = '\0';

   // generate the name of the DSQ file (replace .dsq with .cs)
   char *csFilename = DTS::getFilePath(outputFilename, strlen(shapeName) + 3);
   strcat(csFilename, shapeName);
   strcat(csFilename, ".cs");

   // capitalise first letter for datablock name, then restore for contents
   char c = shapeName[0];
   shapeName[0] = toupper(shapeName[0]);

   // open file and declare datablock
   std::ofstream cs;
   cs.open(csFilename);
   cs << "datablock TSShapeConstructor(" << shapeName << "Dts)\n";
   shapeName[0] = c;    // restore first character

   cs << "{\n";
   cs << "   baseShape = \"./" << shapeName << ".dts\";\n";

   if (mSplitDsq)
   {
      // write each sequence into the datablock
      for (int i = 0; i < mSequences.size(); i++)
      {
         // skip 'Sequence::' identifier at start of name
         const char *seqName = strrchr(mSequences[i]->getName(), ':');
         if (!seqName)
            seqName = mSequences[i]->getName();
         else
            seqName++;

         cs << "   sequence" << i << " = \"./" << shapeName;

            cs << "_" << seqName;
         cs << ".dsq " << seqName << "\";\n";
      }
   }
   else
   {
      // all animations are in the same file
      cs << "   sequence0 = \"./" << shapeName << ".dsq\";\n";
   }
   cs << "};\n";
   cs.close();

   delete [] shapeName;
   delete [] csFilename;
}


void Ms2dtsExporterPlus::exportDone()
{
   // remove auto-generated root bone
   if (rootBone)
   {
      msBone_Destroy(rootBone);
      rootBone = NULL;
      mModel->nNumBones--;
   }

   // remove auto-generated bounds mesh
   if (boundsMesh)
   {
      msMesh_Destroy(boundsMesh);
      boundsMesh = NULL;
      mModel->nNumMeshes--;
   }

   // clear lists
   for (int i = 0; i < mSequences.size(); i++)
      delete mSequences[i];
   for (i = 0; i < mDeletedSequences.size(); i++)
      delete mDeletedSequences[i];
   // mesh objects are deleted during export
   for (i = 0; i < mMaterials.size(); i++)
      delete mMaterials[i];

   mDeletedSequences.clear();
   mSequences.clear();
   mMeshes.clear();
   mMaterials.clear();
}

int Ms2dtsExporterPlus::GetType()
{
   return cMsPlugIn::eTypeExport | cMsPlugIn::eNormalsAndTexCoordsPerTriangleVertex;
}

const char *Ms2dtsExporterPlus::GetTitle()
{
#ifdef _DEBUG
   return "Torque DTS Plus (Debug)...";
#else
   return "Torque DTS Plus...";
#endif
}

int Ms2dtsExporterPlus::Execute(msModel *model)
{
   //--------------------------------------------------------------------------
   // do some basic validity checks
   if (!model)
   {
      alert("Error", "Cannot export NULL model");
      return -1;
   }

   // Milkshape does not like the process directory being changed (texture
   // paths get messed up), so save it and restore at the end of the export
   char currentDir[MaxDlgTextLength];
   ::GetCurrentDirectory(MaxDlgTextLength, currentDir);

   //--------------------------------------------------------------------------
   // read model into node lists
   mModel = model;
   theExporter = this;
   MilkshapeNode::setModel(model);

   readExportOptions();
   readSpecialMaterialList();
   readMeshList();

   // display export dialog box
   if (::DialogBox(hInstance, MAKEINTRESOURCE(IDD_EXPORT), ::GetActiveWindow(), ExportDlgProc) == IDCANCEL)
   {
      ::SetCurrentDirectory(currentDir);
      exportDone();
      return -1;
   }

   // don't export an empty DTS shape
   if (!mExportDSQ && (msModel_GetMeshCount(model) == 0))
   {
      alert("Error", "The model does not contain any geometry - nothing to export.");
      ::SetCurrentDirectory(currentDir);
      exportDone();
      return -1;
   }

   //--------------------------------------------------------------------------
   // get the output file name
   OPENFILENAME ofn ;
   char outputFilename[MaxDlgTextLength+1] = "";

   ::ZeroMemory (&ofn, sizeof(ofn));
   ofn.lStructSize = sizeof(ofn);
   ofn.hInstance   = ::GetModuleHandle(NULL);
   ofn.lpstrFilter = mExportDSQ  ?  "DSQ Files\0*.dsq\0All\0*.*\0" :
                                    "DTS Files\0*.dts\0All\0*.*\0";
   ofn.lpstrFile   = outputFilename;
   ofn.Flags       = OFN_PATHMUSTEXIST | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT;
   ofn.nMaxFile    = MaxDlgTextLength;
   ofn.lpstrDefExt = mExportDSQ ? "dsq" : "dts";

   if (::GetSaveFileName(&ofn) == 0)
   {
      ::SetCurrentDirectory(currentDir);
      exportDone();
      return -1;
   }

   // show the progress dialog
   HWND hProgressDlg = ::CreateDialog(hInstance, MAKEINTRESOURCE(IDD_PROGRESS),
                                       ::GetActiveWindow(), ProgressDlgProc);
   ::ShowWindow(hProgressDlg, SW_SHOW);

   //--------------------------------------------------------------------------
   // start out with milkshape default config
   ::SetDlgItemText(hProgressDlg, IDC_PROGRESS_TEXT, "Reading config...");
   ::SendDlgItemMessage(hProgressDlg, IDC_PROGRESS_BAR, PBM_STEPIT, 0, 0);

   DTS::MsConfig defConfig;
   DTS::AppConfig::setConfig(&defConfig);

   // set up the dump file
   if (mGenDump && !DTS::AppConfig::SetDumpFile(outputFilename, NULL))
      alert("Dump file error", DTS::avar("Unable to create dumpfile for shape \"%s\".", outputFilename));

   if (mUseConfig)
   {
      // read config file if it exists...
      char *fileBase = DTS::getFileBase(outputFilename);
      char *configName = new char[strlen(fileBase) + 5];
      strcpy(configName, fileBase);
      char *ext = strrchr(configName,'.');
      if (!ext)
         ext = configName + strlen(configName);
      strcpy(ext, ".cfg");
      DTS::AppConfig::ReadConfigFile(configName);
      delete [] configName;
   }

   // overwrite config with preferences set in export dialog box
   defConfig.SetEnableSequences(mExportAnim || mExportDSQ);

   //--------------------------------------------------------------------------
   // create bounding box and Root bone
   ::SetDlgItemText(hProgressDlg, IDC_PROGRESS_TEXT, "Creating bounds mesh and root bone...");
   ::SendDlgItemMessage(hProgressDlg, IDC_PROGRESS_BAR, PBM_STEPIT, 0, 0);

   createBounds();

   //--------------------------------------------------------------------------
   // setup bone indices for all meshes now that the Root bone is guaranteed to
   // exist
   for (int i = 0; i < mMeshes.size(); i++)
      mMeshes[i]->setBoneIndices();

   //--------------------------------------------------------------------------
   // enumerate scene and do export
   ::SetDlgItemText(hProgressDlg, IDC_PROGRESS_TEXT, "Enumerating scene...");
   ::SendDlgItemMessage(hProgressDlg, IDC_PROGRESS_BAR, PBM_STEPIT, 0, 0);

   DTS::MsSceneEnum MsSceneEnum(this);

   ::SetDlgItemText(hProgressDlg, IDC_PROGRESS_TEXT, "Processing scene...");
   ::SendDlgItemMessage(hProgressDlg, IDC_PROGRESS_BAR, PBM_STEPIT, 0, 0);

   DTS::Shape *shape = MsSceneEnum.processScene();

   ::SetDlgItemText(hProgressDlg, IDC_PROGRESS_TEXT, "Saving shape...");
   ::SendDlgItemMessage(hProgressDlg, IDC_PROGRESS_BAR, PBM_STEPIT, 0, 0);

   if (!DTS::AppConfig::IsExportError())
   {
      std::ofstream os;

      if (mExportDSQ && mSplitDsq)
      {
         char *p = strrchr(outputFilename, '.');
         *p++ = '_';
         *p = '.';

         for (int i = 0; i < mSequences.size(); i++)
         {
            // skip 'Sequence::' identifier at start of name
            const char *seqName = strrchr(mSequences[i]->getName(), ':');
            if (!seqName)
               seqName = mSequences[i]->getName();
            else
               seqName++;

            // generate name of DSQ file
            strcpy(p, seqName);
            strcat(outputFilename, ".dsq");
      
            os.open(outputFilename, std::ofstream::binary);
            shape->saveSequence(os, i);
            os.close();
         }

         // restore '.' and terminate string (so .cs file will have correct name)
         *p-- = '\0';
         *p = '.';
      }
      else
      {
         os.open(outputFilename, std::ofstream::binary);
         if (mExportDSQ)
            shape->saveSequence(os, -1);
         else
            shape->save(os);
         os.close();
      }

      // generate CS file if exporting DSQ
      if (mExportDSQ && mGenCS)
         generateCsFile(outputFilename);
   }
   DTS::AppConfig::CloseDumpFile();
   delete shape;

   //--------------------------------------------------------------------------
   // clean up
   ::SetDlgItemText(hProgressDlg, IDC_PROGRESS_TEXT, "Copying textures...");
   ::SendDlgItemMessage(hProgressDlg, IDC_PROGRESS_BAR, PBM_STEPIT, 0, 0);

   ::SetCurrentDirectory(currentDir);

   if (DTS::AppConfig::IsExportError())
   {
      alert("Export error", DTS::AppConfig::GetExportError());
   }
   else if (mCopyTextures)
   {
      // get export directory
      char *exportDir = DTS::getFilePath(outputFilename, MS_MAX_NAME+1);
      char *destFilename = exportDir + strlen(exportDir);

      // copy all texture files to the export directory
      for (int i = 0; i < mMaterials.size(); i++)
      {
         if (mMaterials[i]->mRefCount == 0)
            continue;

         // get texture name and existing path
         msMaterial *msMat = mMaterials[i]->getMsMaterial();
         char relPath[MS_MAX_PATH + 1];
         msMaterial_GetDiffuseTexture(msMat, relPath, MS_MAX_PATH);
         char *texName = DTS::getFileBase(relPath);

         // generate source texture path from relative path
         char srcFilename[MS_MAX_PATH+1];
         if (_fullpath(srcFilename, relPath, MS_MAX_PATH))
         {
            // generate destination texture path
            strncpy(destFilename, texName, MS_MAX_NAME);

            // copy the file
            if (strcmp(srcFilename, exportDir))
               ::CopyFile(srcFilename, exportDir, FALSE);
         }
         else
            alert(texName, "Failed to copy texture file!");
      }

      delete [] exportDir;
   }

   ::SetDlgItemText(hProgressDlg, IDC_PROGRESS_TEXT, "Cleanup up after export...");
   ::SendDlgItemMessage(hProgressDlg, IDC_PROGRESS_BAR, PBM_STEPIT, 0, 0);

   exportDone();

   ::DestroyWindow(hProgressDlg);

   return 0;
}

void Ms2dtsExporterPlus::alert(const char * title, const char * message) const
{
   ::MessageBox(::GetActiveWindow(), message, title, MB_OK);
}
