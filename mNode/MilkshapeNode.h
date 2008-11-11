
#ifndef _MILKSHAPENODE_H_
#define _MILKSHAPENODE_H_


// STL
#include <vector>
#include <map>
#include <string>

// milkshape sdk
#include "../msLib/msLib.h"
#include "../msLib/msPlugIn.h"

// dts sdk
#include "DTSQuaternion.h"
#include "DTSMatrix.h"
#include "DTSTypes.h"
#include "DTSPlusTypes.h"


#define MAX_COMMENT_LENGTH 2048


namespace MsDefaults
{
   // exporter options
   const F32 modelScale          = 0.1f;
   const F32 frameRate           = 30.0f;
   const bool cyclic             = false;
};


/// Check if one string begins with another (ignoring case)
///
/// @param s1   String to check the start of
/// @param s2   String to compare against
///
/// @return True if the second string occurs at the start of the first string
inline bool strStartsWith(const char *s1, const char *s2)
{
   return (strnicmp(s1, s2, strlen(s2)) == 0);
}

/// Remove a material from the milkshape model. This is a function that
/// [b]should[/b] have been provided by the milkshape SDK, so it is designed to
/// match the same calling convention.
///
/// @param   pModel      Model to remove the material from
/// @param   pMaterial   Material to remove
void msMaterial_Destroy(msModel *pModel, msMaterial *pMaterial);


//--------------------------------------------------------------------------
/// Helper class to convert a milkshape 3D point to a 3space 3D point
struct MilkshapePoint : public DTS::Point3D
{
   MilkshapePoint(const msVec3 p) : DTS::Point3D(-p[0], p[2], p[1]) {}
};

/// Helper class to convert a set of milkshape angles to a 3space quaternion
struct MilkshapeQuaternion : public DTS::Quaternion
{
   // Torque uses column vectors which means we need to flip the rotations
   MilkshapeQuaternion(const msVec3 p) : DTS::Quaternion(p[0], -p[2], -p[1]) {}
};


//--------------------------------------------------------------------------
/// Wrapper class to mimic the 3ds max 'INode'. All milkshape objects (bones,
/// meshes, materials, sequences etc) are wrapped in an instance of this class.
/// When objects are constructed, various parameters are extracted from the
/// milkshape object comment string and stored as user properties.
class MilkshapeNode
{
protected:

   static msModel *mModel;    //!< All nodes refer to the same milkshape model
   static F32 mScale;         //!< Scale factor applied to exported model

   std::string mName;                     //!< Name of this node
   std::vector<MilkshapeNode*> mChildren; //!< Children nodes
   MilkshapeNode *mParent;                //!< Parent node

   // user properties
   std::map<std::string, F32> mFloatMap;  //!< Floating point properties
   std::map<std::string, S32> mIntMap;    //!< Integer properties
   std::map<std::string, bool> mBoolMap;  //!< Boolean properties

public:

   MilkshapeNode(const char *name = "Unnamed");
   virtual ~MilkshapeNode();

   /// Sets the milkshape model shared by all nodes
   ///
   /// @param model   Pointer to the model
   static void setModel(msModel *model) { mModel = model; }

   /// Get a pointer to the milkshape model
   ///
   /// @return A pointer to the model
   static msModel *getModel() { return mModel; }

   /// Get the model scale factor
   ///
   /// @return The scale factor
   static F32 getScale() { return mScale; }

   /// Set the model scale factor
   ///
   /// @param scale   The scale factor
   static void setScale(F32 scale) { mScale = scale; }

   //--------------------------------------------------------------------------

   /// Returns whether this node represents a milkshape material
   ///
   /// @return True if the node is a material, false if not
   virtual bool isMaterial() const { return false; }

   /// Returns whether this node represents a milkshape mesh
   ///
   /// @return True if the node is a mesh, false if not
   virtual bool isMesh() const { return false; }

   /// Returns whether this node represents a milkshape sequence
   ///
   /// @return True if the node is a sequence, false if not
   virtual bool isSequence() const { return false; }

   /// Returns whether this node represents a milkshape bone
   ///
   /// @return True if the node is a bone, false if not
   virtual bool isBone() const { return false; }

   /// Returns whether this node is the root node
   ///
   /// @return True if the node is the root node, false if not
   bool isRootNode() const { return false; }

   /// Returns whether this node is identical to the specified node
   ///
   /// @param node   Node to compare to
   ///
   /// @return True if the nodes are equal, false if not
   virtual bool isEqual(const MilkshapeNode *node) const { return (node == this); }

   //--------------------------------------------------------------------------

   /// Returns the number of children attached to this node
   ///
   /// @return The number of children attached to this node
   S32 getNumChildren() const { return mChildren.size(); }

   /// Adds a child node
   ///
   /// @param child   Child node to add
   void addChildNode(MilkshapeNode *child);

   /// Retrieve the indexed child node
   ///
   /// @param idx   Index of the child node to retrieve
   ///
   /// @return The children node at the specified index, NULL if not found
   MilkshapeNode *getChildNode(S32 idx) const;

   /// Sets the parent of this node
   ///
   /// @param parent   Pointer to the parent of this node
   void setParentNode(MilkshapeNode *parent) { mParent = parent; }

   /// Returns a pointer to this nodes parent node
   ///
   /// @return The parent node, NULL if this node has no parent
   MilkshapeNode *getParentNode() const { return mParent; }

   //--------------------------------------------------------------------------

   /// Get the name of this node
   ///
   /// @return The name of this node
   const char *getName() const { return mName.data(); }

   /// Set the name of this node
   ///
   /// @param name   The new name for this node
   void setName(const char *name) { mName = name; }

   //--------------------------------------------------------------------------

   /// Get the transform of this node at a certain animation frame
   ///
   /// @param frame   Index of the animation frame at which to retrieve the node transform
   ///
   /// @return The nodes transform at the given frame
   virtual DTS::Matrix<4,4,F32> getNodeTransform(S32 frame) const;

   /// Get the visibility of this node at a certain frame
   ///
   /// @param frame   Index of the animation frame at which to retrieve the visibility info
   ///
   /// @return Visibility from 0.0 (invisible) to 1.0 (opaque)
   virtual F32 getVisibility(S32 frame) const { return 1.0f; }

   //--------------------------------------------------------------------------

   /// Write user properties to a formatted string. Note that the maxLength is
   /// not really adhered to, but simply asserted at the end of the write.
   ///
   /// @param buff        String to write to.
   /// @param maxLength   Maximum number of characters to write to the string
   void writePropertyString(char *buff, int maxLength) const;

   /// Read user properties from a formatted string. No validation of valid
   /// input format is performed!
   ///
   /// @param buff   String to read
   void readPropertyString(char *buff);

   //--------------------------------------------------------------------------

   /// Get a floating point property value
   ///
   /// @param propName   Name of the property to get
   /// @param val        Reference to variable to hold return value
   ///
   /// @return True if a value was set, false if not
   bool getUserPropFloat(const char *propName, F32 &val) const;

   /// Get an integer property value
   ///
   /// @param propName   Name of the property to get
   /// @param val        Reference to variable to hold return value
   ///
   /// @return True if a value was set, false if not
   bool getUserPropInt(const char *propName, S32 &val) const;

   /// Get a boolean property value
   ///
   /// @param propName   Name of the property to get
   /// @param val        Reference to variable to hold return value
   ///
   /// @return True if a value was set, false if not
   bool getUserPropBool(const char *propName, bool &val) const;

   //--------------------------------------------------------------------------

   /// Set a floating point property value. If the property does not exist, it
   /// will be created.
   ///
   /// @param propName   Name of the property to set
   /// @param val        Value to set
   void setUserPropFloat(const char *propName, F32 val) { mFloatMap[propName] = val; }

   /// Set an integer property value. If the property does not exist, it will
   /// be created.
   ///
   /// @param propName   Name of the property to set
   /// @param val        Value to set
   void setUserPropInt(const char *propName, S32 val) { mIntMap[propName] = val; }

   /// Set a boolean property value. If the property does not exist, it will be
   /// created.
   ///
   /// @param propName   Name of the property to set
   /// @param val        Value to set
   void setUserPropBool(const char *propName, bool val) { mBoolMap[propName] = val; }
};

#endif // _MILKSHAPENODE_H_
