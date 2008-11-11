
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "MilkshapeBone.h"

using namespace DTS;


MilkshapeBone::MilkshapeBone(S32 boneIndex) : MilkshapeNode("bone")
{
   char boneName[MS_MAX_NAME+1];
   msBone *bone = msModel_GetBoneAt(mModel, boneIndex);

   assert(bone && "Could not find bone!");

   msBone_GetName(bone, boneName, MS_MAX_NAME);
   setName(boneName);

   mBoneIndex = boneIndex;
}

bool MilkshapeBone::isEqual(const MilkshapeNode *node) const
{
   const MilkshapeBone *boneNode = dynamic_cast<const MilkshapeBone*>(node);
   return (boneNode && (mBoneIndex == boneNode->mBoneIndex));
}

void MilkshapeBone::lerpTranslation(Point3D *out, const Point3D &a,
                                    const Point3D &b, float alpha) const
{
   out->x(a.x() + (b.x()-a.x()) * alpha);
   out->y(a.y() + (b.y()-a.y()) * alpha);
   out->z(a.z() + (b.z()-a.z()) * alpha);
}

void MilkshapeBone::lerpTranslation(Point3D *out, const Point3D &a, const Point3D &b,
                                    int outFrame, int aFrame, int bFrame) const
{
   float alpha = 0.f;

   // check for dt == 0
   if(bFrame != aFrame)
      alpha = (float)(outFrame - aFrame) / (float)(bFrame - aFrame);

   lerpTranslation(out, a, b, alpha);
}

void MilkshapeBone::lerpRotation(Quaternion *out, const Quaternion &q1,
                                 const Quaternion &q2, float alpha) const
{
   //-----------------------------------
   // calculate the cosine of the angle:
   double cosOmega = q1.x() * q2.x() + q1.y() * q2.y() + q1.z() * q2.z() + q1.w() * q2.w();

   //-----------------------------------
   // adjust signs if necessary:
   float sign2;
   if ( cosOmega < 0.0f )
   {
      cosOmega = -cosOmega;
      sign2 = -1.0f;
   }
   else
      sign2 = 1.0f;

   //-----------------------------------
   // calculate interpolating coeffs:
   double scale1, scale2;
   if ( (1.0f - cosOmega) > 0.00001f )
   {
      // standard case
      double omega = acos(cosOmega);
      double sinOmega = sin(omega);
      scale1 = sin((1.0f - alpha) * omega) / sinOmega;
      scale2 = sign2 * sin(alpha * omega) / sinOmega;
   }
   else
   {
      // if quats are very close, just do linear interpolation
      scale1 = 1.0f - alpha;
      scale2 = sign2 * alpha;
   }

   //-----------------------------------
   // actually do the interpolation:
   out->x( float(scale1 * q1.x() + scale2 * q2.x()));
   out->y( float(scale1 * q1.y() + scale2 * q2.y()));
   out->z( float(scale1 * q1.z() + scale2 * q2.z()));
   out->w( float(scale1 * q1.w() + scale2 * q2.w()));
}

void MilkshapeBone::lerpRotation(Quaternion *out, const Quaternion &a, const Quaternion &b,
                                 int outFrame, int aFrame, int bFrame) const
{
   float alpha = 0.f;

   // check for dt == 0
   if(bFrame != aFrame)
      alpha = (float)(outFrame - aFrame) / (float)(bFrame - aFrame);

   lerpRotation(out,a,b,alpha);
}

Point3D MilkshapeBone::getPositionKey(msBone *bone, int frame) const
{
   int numKeys = msBone_GetPositionKeyCount(bone);
   if((frame != 0) && (numKeys > 0))
   {
      //-------------------------------------------------------
      // find keyframe indices either side of the desired frame
      int frame1 = -1;
      int frame2 = numKeys-1;
      for(int i=0; i < numKeys; i++)
      {
         msPositionKey *msKey = msBone_GetPositionKeyAt(bone, i);
         int keyframe = int(msKey->fTime);

         if(keyframe <= frame)
            frame1 = i;

         if(keyframe >= frame)
         {
            if(frame1 == -1)
               frame1 = i;
            frame2 = i;
            break;
         }
      }

      //-----------------------
      // get keyframe positions
      msPositionKey *pKey1 = msBone_GetPositionKeyAt(bone, frame1);
      msPositionKey *pKey2 = msBone_GetPositionKeyAt(bone, frame2);
      Point3D p1 = MilkshapePoint(pKey1->Position);
      Point3D p2 = MilkshapePoint(pKey2->Position);

      //----------------------------------------------
      // interpolate to find position at desired frame
      int time1 = (frame < (int)pKey1->fTime) ? frame : (int)pKey1->fTime;
      int time2 = (frame > (int)pKey2->fTime) ? frame : (int)pKey2->fTime;
      Point3D pt;
      lerpTranslation(&pt, p1, p2, frame, time1, time2);
      return pt;
   }
   else
      return Point3D(0,0,0);
}

Quaternion MilkshapeBone::getRotationKey(msBone *bone, int frame) const
{
   int numKeys = msBone_GetRotationKeyCount(bone);
   if((frame != 0) && (numKeys > 0))
   {
      //-------------------------------------------------------
      // find keyframe indices either side of the desired frame
      int frame1 = -1;
      int frame2 = numKeys-1;
      for(int i=0; i < numKeys; i++)
      {
         msRotationKey *msKey = msBone_GetRotationKeyAt(bone, i);
         int keyframe = int(msKey->fTime);

         if(keyframe <= frame)
            frame1 = i;

         if(keyframe >= frame)
         {
            if(frame1 == -1)
               frame1 = i;
            frame2 = i;
            break;
         }
      }

      //-----------------------
      // get keyframe rotations
      msRotationKey *qKey1 = msBone_GetRotationKeyAt(bone, frame1);
      msRotationKey *qKey2 = msBone_GetRotationKeyAt(bone, frame2);
      Quaternion q1 = MilkshapeQuaternion(qKey1->Rotation);
      Quaternion q2 = MilkshapeQuaternion(qKey2->Rotation);

      //----------------------------------------------
      // interpolate to find rotation at desired frame
      int time1 = (frame < (int)qKey1->fTime) ? frame : (int)qKey1->fTime;
      int time2 = (frame > (int)qKey2->fTime) ? frame : (int)qKey2->fTime;
      Quaternion qt;
      lerpRotation(&qt, q1, q2, frame, time1, time2);
      return qt;
   }
   else
      return Quaternion::identity;
}

void MilkshapeBone::getBonePosRot(msBone *bone, int frame, Point3D& pos,
                                    Quaternion& rot) const
{
   assert(bone && "NULL bone passed to MilkshapeBone::getBonePosRot");

   // check for parent bone
   char parentName[MS_MAX_NAME+1];
   msBone_GetParentName(bone, parentName, MS_MAX_NAME);
   int parentIndex = msModel_FindBoneByName(mModel, parentName);

   if(parentIndex >= 0)
   {
      // get the parent bone orientation
      msBone *parentBone = msModel_GetBoneAt(mModel, parentIndex);
      getBonePosRot(parentBone, frame, pos, rot);
   }

   // get base (frame 0) transform
   msVec3 vec;
   msBone_GetPosition(bone, vec);
   Point3D p = MilkshapePoint(vec);
   msBone_GetRotation(bone, vec);
   Quaternion q = MilkshapeQuaternion(vec);

   if(frame != 0)
   {
      // get milkshape position/rotation.....do separately, as there may not
      // necessarily be a position & rotation at each keyframe
      Point3D pt = getPositionKey(bone, frame);
      Quaternion qt = getRotationKey(bone, frame);

      // keyframe position/rotation are relative to base transform, so apply
      // to base transform to get absolute position/rotation
      p += q.apply(pt);
      q = qt * q;
   }

   // bone position/rotation is relative to the parent bone, so apply
   // to parent transform to get position/rotation
   pos += rot.apply(p * mScale);
   rot = q * rot;
}


Matrix<4,4,F32> MilkshapeBone::getNodeTransform(S32 frame) const
{
   assert(mBoneIndex >= 0 && mBoneIndex < msModel_GetBoneCount(mModel)
                           && "Bone index out of range");

   //--------------------------------------------
   // get the absolute bone position and rotation
   msBone *bone = msModel_GetBoneAt(mModel, mBoneIndex);
   Point3D p = Point3D(0,0,0);
   Quaternion q = Quaternion::identity;
   getBonePosRot(bone, frame, p, q);

   //---------------------
   // convert to transform
   Matrix<4,4,F32>::Row posRow;
   posRow[0] = p.x();
   posRow[1] = p.y();
   posRow[2] = p.z();
   posRow[3] = 1.f;

   // q.toMatrix builds a transposed transform from what we
   // want, so we need to pass the inverse quaternion
   Matrix<4,4,F32> ret(q.inverse().toMatrix());
   ret.setCol(3, posRow);

   return ret;
}
