
#ifndef _MSAPPNODE_H_
#define _MSAPPNODE_H_

#include "appNode.h"
#include "MilkshapeNode.h"


namespace DTS
{
   /// Wrapper for a milkshape node
   class MsAppNode : public AppNode
   {
      typedef AppNode Parent;

      MilkshapeNode *mMsNode;    //!< The milkshape node wrapped by this class
      bool mOwnsNode;            //!< True if this class owns the milkshape node

      /// If this node is a mesh, generate the list of meshes that it encompasses
      void buildMeshList();

      /// Generate the list of children nodes
      void buildChildList();

   public:

      MsAppNode(MilkshapeNode *node, bool owner=false);
      ~MsAppNode();

      //-----------------------------------------------------------------------

      /// Check if two nodes are equal
      ///
      /// @param node   Node to compare
      ///
      /// @return True if the nodes are equal, false otherwise
      bool isEqual(AppNode *node);

      /// Generate the transform of this node at a certain time
      ///
      /// @param time   Time at which to generate the transform
      ///
      /// @return The node transform
      Matrix<4,4,F32> getNodeTransform(const AppTime &time);

      /// Check if the bounds node is animated by the input sequence
      ///
      /// @param seqData   Animation sequence to check
      ///
      /// @return True if the sequence animates the bounds node, false otherwise
      bool animatesTransform(const AppSequenceData &seqData);

      //-----------------------------------------------------------------------

      /// Get the name of this node
      ///
      /// @return A character string containing the name of this node
      const char *getName();

      /// Get the name of this nodes parent
      ///
      /// @return A character string containing the name of this nodes parent
      const char *getParentName();

      /// Return whether this nodes parent is the root node
      ///
      /// @return True if this nodes parent is the root node, false otherwise
      bool isParentRoot();

      //-----------------------------------------------------------------------

      /// Get a floating point property value
      ///
      /// @param propName     Name of the property to get
      /// @param defaultVal   Reference to variable to hold return value
      ///
      /// @return True if a value was set, false if not
      bool getFloat(const char *propName, F32 &defaultVal);

      /// Get an integer property value
      ///
      /// @param propName     Name of the property to get
      /// @param defaultVal   Reference to variable to hold return value
      ///
      /// @return True if a value was set, false if not
      bool getInt(const char *propName, S32 &defaultVal);

      /// Get a boolean property value
      ///
      /// @param propName     Name of the property to get
      /// @param defaultVal   Reference to variable to hold return value
      ///
      /// @return True if a value was set, false if not
      bool getBool(const char *propName, bool &defaultVal);
   };

};

#endif // MSAPPNODE_H_

