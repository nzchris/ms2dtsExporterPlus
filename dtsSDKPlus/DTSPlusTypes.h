//-----------------------------------------------------------------------------
// Torque Game Engine 
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#ifndef DTSPLUSTYPES_H_
#define DTSPLUSTYPES_H_

typedef signed char        S8;      // Compiler independent Signed Char
typedef unsigned char      U8;      // Compiler independent Unsigned Char

typedef signed short       S16;     // Compiler independent Signed 16-bit short
typedef unsigned short     U16;     // Compiler independent Unsigned 16-bit short

typedef signed int         S32;     // Compiler independent Signed 32-bit integer
typedef unsigned int       U32;     // Compiler independent Unsigned 32-bit integer

typedef float              F32;     // Compiler independent 32-bit float
typedef double             F64;     // Compiler independent 64-bit float

// Simple template used to make sure some operation
// gets executed before going out of scope
template <class Type> class OnDestroy
{
protected:
   Type * mObj;

   void transfer(const OnDestroy<Type> * des)
   {
      doit();
      if (des)
      {
         mObj = des->mObj;
         const_cast<OnDestroy<Type>*>(des)->mObj=0;
      }
   }

   virtual void doit() = 0;

public:

   OnDestroy(Type * obj=0) { mObj=obj; }
   OnDestroy(const OnDestroy<Type> & des) { transfer(&des); }
   virtual ~OnDestroy() {}
   void operator=(const OnDestroy<Type> & des) { transfer(&des); }
};


#endif // DTSPLUSTYPES_H_

