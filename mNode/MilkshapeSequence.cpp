
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "DTSUtil.h"
#include "MilkshapeSequence.h"


bool MilkshapeSequence::readOldSequenceName(char *name)
{
   bool named = false;
   char *ptr = name + strlen("seq:");
   char buff[MS_MAX_NAME+1];
   for (char *tok = strtok(ptr,","); tok != 0; tok = strtok(0,","))
   {
      // Strip lead/trailing white space
      for (; isspace(*tok); tok++);
      for (char* itr = tok + strlen(tok)-1; itr > tok && isspace(*itr); itr--)
         *itr = 0;

      //
      char start[MS_MAX_NAME+1],end[MS_MAX_NAME+1];
      if (sscanf(tok,"%[a-zA-Z0-9]=%[0-9]-%[0-9]",buff,start,end) == 3)
      {
         mName = buff;
         named = true;

         setUserPropInt("startFrame",atoi(start));
         setUserPropInt("endFrame",atoi(end));

         // clear unsupported attributes
         setUserPropBool("blend",false);
         setUserPropInt("blendReferenceFrame",0);
      }
      else
      {
         int fps,blendRef;
         if (sscanf(tok,"fps=%d",&fps))
            setUserPropFloat("frameRate",(float)fps);
         if (sscanf(tok,"ref=%d",&blendRef))
         {
            setUserPropBool("blend",true);
            setUserPropInt("blendReferenceFrame",blendRef);
         }
         if (!strcmp(tok,"cyclic"))
            setUserPropBool("cyclic",true);
      }
   }

   // valid sequence?
   if (!named)
      return false;

   return true;
}

MilkshapeSequence::MilkshapeSequence(U32 seqIndex, char *name) : MilkshapeNode(name)
{
   // initially set sequence to an invalid index
   mSeqIndex = (U32)-1;

   // set defaults
   setUserPropInt("startFrame",1);
   setUserPropInt("endFrame",2);
   setUserPropFloat("frameRate",MsDefaults::frameRate);
   setUserPropBool("cyclic",MsDefaults::cyclic);
   setUserPropBool("blend",false);
   setUserPropInt("blendReferenceFrame", 1);
   setUserPropFloat("priority", MsDefaults::priority);
   setUserPropFloat("overrideDuration", MsDefaults::overrideDuration);
   setUserPropBool("enableMorph",MsDefaults::enableMorph);
   setUserPropBool("enableTVert",MsDefaults::enableTVert);
   setUserPropBool("enableVis",MsDefaults::enableVis);
   setUserPropBool("enableTransform",MsDefaults::enableTransform);
   setUserPropBool("enableIFL",MsDefaults::enableIFL);
   setUserPropBool("ignoreGround", MsDefaults::ignoreGround);
   setUserPropBool("autoGround", MsDefaults::autoGround);
   setUserPropFloat("groundFrameRate", MsDefaults::groundFrameRate);
   setUserPropFloat("groundXSpeed", MsDefaults::groundXSpeed);
   setUserPropFloat("groundYSpeed", MsDefaults::groundYSpeed);
   setUserPropFloat("groundZSpeed", MsDefaults::groundZSpeed);
   clearTriggers();

   if (!name)
      return;

   // check for old-style sequence name
   if (strStartsWith(name,"seq:"))
   {
      if (readOldSequenceName(name))
         mSeqIndex = seqIndex;
   }
   else if (strStartsWith(name, "*"))
   {
      mSeqIndex = seqIndex;
      mName = name+1;

      // read properties from comment string
      char commentString[MAX_COMMENT_LENGTH+1] = "";
      msMaterial_GetComment(getMsMaterial(), commentString, MAX_COMMENT_LENGTH);
      readPropertyString(commentString);
   }
}

bool MilkshapeSequence::isEqual(const MilkshapeNode *node) const
{
   const MilkshapeSequence *seqNode = dynamic_cast<const MilkshapeSequence*>(node);
   return (seqNode && (getMsMaterial() == seqNode->getMsMaterial()));
}

void MilkshapeSequence::clearTriggers()
{
   int numTriggers = 0;
   getUserPropInt("numTriggers", numTriggers);
   for (int i = 0; i < numTriggers; i++)
   {
      mIntMap.erase(DTS::avar("triggerFrame%d", i));
      mIntMap.erase(DTS::avar("triggerState%d", i));
   }
   setUserPropInt("numTriggers", 0);
}
