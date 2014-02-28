// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "../Include/intermediate.h"
#include "RemoveTree.h"

// Code to recursively delete the intermediate tree.

static void RemoveSymbol(TIntermSymbol* node, TIntermTraverser* it)
{
   node->Release();
}

static bool RemoveBinary(bool  /*preVisit*/ , TIntermBinary* node, TIntermTraverser*)
{
   node->Release();
   return true;
}

static bool RemoveUnary(bool, /*preVisit */ TIntermUnary* node, TIntermTraverser*)
{
   node->Release();
   return true;
}

static bool RemoveAggregate(bool  /*preVisit*/ , TIntermAggregate* node, TIntermTraverser*)
{
   node->Release();
   return true;
}

static bool RemoveSelection(bool  /*preVisit*/ , TIntermSelection* node, TIntermTraverser*)
{
   node->Release();
   return true;
}

static void RemoveConstant(TIntermConstant* node, TIntermTraverser*)
{
   node->Release();
}

void RemoveAllTreeNodes(TIntermNode* root)
{
   TIntermTraverser it;

   it.visitAggregate     = RemoveAggregate;
   it.visitBinary        = RemoveBinary;
   it.visitConstant = RemoveConstant;
   it.visitSelection     = RemoveSelection;
   it.visitSymbol        = RemoveSymbol;
   it.visitUnary         = RemoveUnary;

   it.preVisit = false;
   it.postVisit = true;

   root->traverse(&it);
}
