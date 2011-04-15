#ifndef _MILKSHAPESEQUENCE_H_
#define _MILKSHAPESEQUENCE_H_


#include "MilkshapeNode.h"


namespace MsDefaults
{
   // animation settings
   const F32 priority            = 5;
   const F32 overrideDuration    = -1;
   const S32 numTriggers         = 0;
   const bool enableMorph        = false;
   const bool enableTVert        = false;
   const bool enableVis          = false;
   const bool enableTransform    = true;
   const bool enableIFL          = false;
   const bool ignoreGround       = false;
   const bool autoGround         = false;
   const F32 groundFrameRate     = 10;
   const F32 groundXSpeed        = 0;
   const F32 groundYSpeed        = 0;
   const F32 groundZSpeed        = 0;
}


//--------------------------------------------------------------------------
/// Wrapper class for a milkshape sequence
class MilkshapeSequence : public MilkshapeNode
{
   enum
   {
      // animation flags
      AnimCyclic           = 1 << 0,   //!< This animation will loop until stopped
      AnimBlend            = 1 << 1,   //!< This animation is blended
      AnimIgnoreGround     = 1 << 2,   //!< Don't export a ground transform for this sequence
      AnimEnableMorph      = 1 << 3,   //!< Enables morph animation
      AnimEnableTVert      = 1 << 4,   //!< Enables animated texture coordinates
      AnimEnableVis        = 1 << 5,   //!< Enables visibility channel
      AnimEnableTransform  = 1 << 6,   //!< Enables transform animation
      AnimEnableIfl        = 1 << 7,   //!< Enables IFL material animation
   };

   U32 mSeqIndex;          //!< Index of the milkshape special material object

   //--------------------------------------------------------------------------

   /// Reads an old-style milkshape sequence name. Provided for backwards
   /// compatibility with the original ms2dtsExporter. Old-style sequence names
   /// were of the form:<BR>
   /// Seq: ambient=2-3, cyclic, fps=2, ref=1
   ///
   /// @param name   The name to convert
   ///
   /// @return True if the name was converted successfully, false otherwise
   bool readOldSequenceName(char *name);

public:
   enum
   {
      MaxTriggers      = 32,  //!< Maximum number of triggers per sequence
   };

   MilkshapeSequence(U32 seqIndex=(U32)-1, char *name=0);

   bool isEqual(const MilkshapeNode *node) const;
   bool isSequence() const { return true; }

   //--------------------------------------------------------------------------

   /// Get the index of the milkshape material used by this sequence
   ///
   /// @return The index of the milkshape material used by this sequence
   U32 getMatIndex() const { return mSeqIndex; }

   /// Set the index of the milkshape material used by this sequence
   ///
   /// @param matIndex   The index of the milkshape material to use
   void setMatIndex(U32 matIndex) { mSeqIndex = matIndex; }

   /// Get the milkshape material used by this sequence.
   ///
   /// @return A pointer to the milkshape material (NULL if index is bad)
   msMaterial *getMsMaterial() const
   {
      if ((int)mSeqIndex < msModel_GetMaterialCount(mModel))
         return msModel_GetMaterialAt(mModel, mSeqIndex);
      else
         return NULL;
   }

   //--------------------------------------------------------------------------

   /// Remove all trigger keyframes
   void clearTriggers();
};

#endif // _MILKSHAPESEQUENCE_H_
