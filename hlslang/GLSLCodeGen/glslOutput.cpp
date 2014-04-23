// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "glslOutput.h"

#include <cstdlib>

#ifdef _WIN32
	#define snprintf _snprintf
#endif

bool isShadowSampler(TBasicType t) {
	switch (t) {
		case EbtSampler1DShadow:
		case EbtSampler2DShadow:
		case EbtSamplerRectShadow:
			return true;
		default:
			return false;
	}
}

int getElements( EGlslSymbolType t )
{
   switch (t)
   {
   case EgstBool:
   case EgstInt:
   case EgstFloat:
   case EgstStruct:
      return 1;
   case EgstBool2:
   case EgstInt2:
   case EgstFloat2:
      return 2;
   case EgstBool3:
   case EgstInt3:
   case EgstFloat3:
      return 3;
   case EgstBool4:
   case EgstInt4:
   case EgstFloat4:
   case EgstFloat2x2:
      return 4;
   case EgstFloat3x3:
      return 9;
   case EgstFloat4x4:
      return 16;
   }

   return 0;
}

TString buildArrayConstructorString(const TType& type) {
	std::stringstream constructor;
	constructor << getTypeString(translateType(&type))
				<< '[' << type.getArraySize() << ']';

	return TString(constructor.str().c_str());
}


void writeConstantConstructor( std::stringstream& out, EGlslSymbolType t, TPrecision prec, TIntermConstant *c, GlslStruct *structure = 0 )
{
	unsigned n_elems = getElements(t);
	bool construct = n_elems > 1 || structure != 0;

	if (construct) {
		writeType (out, t, structure, EbpUndefined);
		out << "(";
	}
	
	if (structure) {
		// compound type
		unsigned n_members = structure->memberCount();
		for (unsigned i = 0; i != n_members; ++i) {
			const GlslStruct::StructMember &m = structure->getMember(i);
			if (construct && i > 0)
				out << ", ";
			writeConstantConstructor (out, m.type, m.precision, c);
		}
	} else {
		// simple type
		unsigned n_constants = c->getCount();
		for (unsigned i = 0; i != n_elems; ++i) {
			unsigned v = Min(i, n_constants - 1);
			if (construct && i > 0)
				out << ", ";
			
			switch (c->getBasicType()) {
				case EbtBool:
					out << (c->toBool(v) ? "true" : "false");
					break;
				case EbtInt:
					out << c->toInt(v);
					break;
				case EbtFloat:
					GlslSymbol::writeFloat(out, c->toFloat(v));
					break;
				default:
					assert(0);
			}
		}
	}

	if (construct)
		out << ")";
}


void writeTempVarDecl(const TType *type, const char* tempVarName, TGlslOutputTraverser* goit, std::stringstream& out)
{
	if (type->getBasicType() == EbtStruct)
		out << type->getTypeName();
	else
	{
		EGlslSymbolType symbol_type = translateType(type);
		writeType(out, symbol_type, NULL, goit->m_UsePrecision ? type->getPrecision() : EbpUndefined);
	}

	out << " " << tempVarName;
	if (type->isArray())
		out << "[" << type->getArraySize() << "]";
}

void writeTempVarAssign(const TType *type, const char* tempVarName, TGlslOutputTraverser* goit, std::stringstream& out)
{
	if (type->getBasicType() == EbtStruct)
		out << type->getTypeName();
	else
	{
		EGlslSymbolType symbol_type = translateType(type);
		writeType(out, symbol_type, NULL, goit->m_UsePrecision ? type->getPrecision() : EbpUndefined);
	}

	out << " " << tempVarName;
	if (type->isArray())
		out << "[" << type->getArraySize() << "]";
	out << " = ";
}

void writeTempVarAssign(const TType *type, const std::string& tempVarName, TGlslOutputTraverser* goit, std::stringstream& out)
{
#if defined DEBUG || defined _DEBUG
	bool dbgbreak = tempVarName.compare("xlat_bintemp45") == 0;
#endif
	writeTempVarAssign(type, tempVarName.c_str(), goit, out);
}

//----------------write an operand which is an element of uniform non-square matrix( uniform non-square matrix is transformed to an array of vector)------------------
void writeUniformNQMArrayElementOperand(const TType* type, const char* varName, const char* indexEpr, std::stringstream& out)
{
	out << type->getTypeName();
	out << "(";
	for (int col = 0; col < type->getNonSquareColumns(); ++col)
	{
		if (col > 0)
			out << ", ";
		out << varName << "[" << col << " + " << indexEpr << " * " << type->getNonSquareColumns() << "]";
	}
	out << ")";
}

void writeComparison( const TString &compareOp, const TString &compareCall, TIntermBinary *node, TGlslOutputTraverser* goit ) 
{
   GlslFunction *current = goit->current;    
   std::stringstream& out = current->getActiveOutput();
   bool l_parentRequireValue = goit->parentRequireValue;
   bool bUseCompareCall = false;

   // Determine whether we need the vector or scalar comparison function
   if ( ( node->getLeft() && node->getLeft()->getNominalSize() > 1 ) ||
        ( node->getRight() && node->getRight()->getNominalSize() > 1 ) )
   {
      bUseCompareCall = true;
   }

   //---------------------------------
   std::string leftOut;
   std::stringstream childOutStream;//this also be used as rightOut
   bool needTempVar = false;
   //----traverse left and right-------------------
	goit->parentRequireValue = true;//require them return values
	//left
	goit->tempVariableName = "";
	current->setActiveOutput(&childOutStream);
	if (node->getLeft())
	{
		node->getLeft()->traverse(goit);	
		if (goit->tempVariableName.size() > 0)//we have temporary variable
		{
			out << childOutStream.str();//write its calculation first
			leftOut = goit->tempVariableName;
			needTempVar = true;
		}
		else
			leftOut = childOutStream.str();
	}
	//right
	goit->tempVariableName = "";
	childOutStream.str("");//clear right's output stream
	current->setActiveOutput(&childOutStream);
	if (node->getRight())
	{
		node->getRight()->traverse(goit);	
		if (goit->tempVariableName.size() > 0)//we have temporary variable
		{
			out << childOutStream.str();//write its calculation first
			childOutStream.str(goit->tempVariableName);
			needTempVar = true;
		}
	}
	//----------------------
	
	current->setActiveOutput(&out);
	goit->tempVariableName = "";
	
	needTempVar = needTempVar && l_parentRequireValue;//really need temp var ?
	current->beginStatement();
	if (needTempVar)
	{
		char *tempVar = new char[64];
		snprintf(tempVar, 64, "xlat_bintemp%d", goit->swizzleAssignTempCounter++);
		goit->tempVariableName = tempVar;
		delete[] tempVar;

		writeTempVarAssign(node->getTypePointer(), goit->tempVariableName, goit, out);
	}

   // Output vector comparison
   if ( bUseCompareCall )
   {
      out << compareCall << "( ";

      if (node->getLeft())
      {
         // If it is a float, need to smear to the size of the right hand side
         if ( node->getLeft()->getNominalSize() == 1 )
         {
            out << "vec" <<  node->getRight()->getNominalSize() << "( ";

            out << leftOut;

            out << " )";                
         }
         else
         {
            out << leftOut;
         }         
      }
      out << ", ";

      if (node->getRight())
      {
         // If it is a float, need to smear to the size of the left hand side
         if ( node->getRight()->getNominalSize() == 1 )
         {
            out << "vec" <<  node->getLeft()->getNominalSize() << "( ";

            out << childOutStream.str();

            out << " )";             
         }
         else
         {
            out << childOutStream.str();
         }         
      }
      out << ")";
   }
   // Output scalar comparison
   else
   {
      out << "(";

      if (node->getLeft())
         out << leftOut;
      out << " " << compareOp << " ";
      if (node->getRight())
         out << childOutStream.str();

      out << ")";
   }

   if (needTempVar)
	   current->endStatement();

   goit->parentRequireValue = l_parentRequireValue;
}


void writeFuncCall( const TString &name, TIntermAggregate *node, TGlslOutputTraverser* goit, bool bGenMatrix = false )
{
   TIntermSequence::iterator sit;
   TIntermSequence &sequence = node->getSequence(); 
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();
   bool l_parentRequireValue = goit->parentRequireValue;
   bool l_parentInlining = goit->isInlining;
   bool isFuncReturn = node->getTypePointer()->getBasicType() != EbtVoid;//does this function return something

   std::vector<std::string> paramsOut;

   bool needTempVar = false;

   goit->parentRequireValue = true;//require children node(sequence returns a value)

   for (sit = sequence.begin(); sit != sequence.end(); ++sit)
	{
		goit->tempVariableName = "";
		std::stringstream paramOutStream;
		current->setActiveOutput(&paramOutStream);
		(*sit)->traverse(goit);

		if (goit->tempVariableName.size() > 0)//we have temporary variable holding data calculated from last traversal
		{
			out << paramOutStream.str();//write the temporary variable calculation first
			paramsOut.push_back(goit->tempVariableName);//use this temporary variable as a parameter
			needTempVar = true;//we need new temp variable to hold the return value of this function call
		}
		else
			paramsOut.push_back(paramOutStream.str());
	}

   current->setActiveOutput(&out);

   current->beginStatement();

   needTempVar = l_parentRequireValue && needTempVar && isFuncReturn;
   
   //check if this is inline function
   std::map<TString, TIntermAggregate*>::iterator funcIte = goit->inlinefuncList.find(node->getName());
   if (funcIte != goit->inlinefuncList.end())
   {
	   TIntermAggregate *funcNode = funcIte->second;
	   TIntermNode *funcBody = NULL;
	   //store old values
	   std::string *parentRetVar = goit->inlineRetVar;
	   std::map<int, std::string>  *parentParamsMap = goit->inlineParamsMap;

	   //write declaration of temporary variable that will hold passed parameter value
	   std::map<int, std::string> newParamsMap;
	   char * varName = new char[64];
		int paramIndex = 0;
		TIntermSequence* parameters = NULL;
		TIntermSequence::iterator paramIte;
		if (funcNode->getSequence().size() > 1)
		{
			parameters = &(funcNode->getSequence()[0]->getAsAggregate()->getSequence());
			paramIte = parameters->begin();

			funcBody = funcNode->getSequence()[1];
		}
		else
			funcBody = funcNode->getSequence()[0];

		for (sit = sequence.begin(); sit != sequence.end(); ++sit, ++ paramIndex, ++paramIte)
		{
			TIntermSymbol *symbol = (*paramIte)->getAsSymbolNode();//get parameter symbol 
			if ( symbol->getQualifier() == EvqInOut || symbol->getQualifier() == EvqOut)//reference parameter, just use the parameter
			{
				newParamsMap[symbol->getId()] = paramsOut[paramIndex];
			}
			else
			{
				snprintf(varName, 64, "xlat_inlinevartemp%d", goit->swizzleAssignTempCounter ++);//create temp var name
				newParamsMap[symbol->getId()] = varName;//store it in parameters map

				//declare temp var and assign it
				current->beginStatement();
				writeTempVarAssign(symbol->getTypePointer(), varName, goit, out);
				out << paramsOut[paramIndex];
				current->endStatement();
			}
		}
		//write declaration of temp var that will hold returned value
		std::string newRetVar;
		if (isFuncReturn)
		{
			snprintf(varName, 64, "xlat_inlinerettemp%d", goit->swizzleAssignTempCounter ++);//create temp var name
			newRetVar = varName;
			//declare temp var that will hold the return value
			current->beginStatement();
			writeTempVarDecl(node->getTypePointer(), varName, goit, out);
			current->endStatement();
		}
		delete [] varName;

		goit->isInlining = true;
		goit->inlineParamsMap = &newParamsMap;
		goit->inlineRetVar = &newRetVar;
		//start inline expanding
		//use its own scope
		current->beginStatement();
		current->beginBlock();
		funcBody->traverse(goit);
		current->endBlock();
		current->endStatement();


		//restore old values
		goit->inlineParamsMap = parentParamsMap;
		goit->inlineRetVar = parentRetVar;
		goit->tempVariableName = newRetVar;
   }
   else//normal function call
   {
	   if (needTempVar)
	   {
		   char *tempVar = new char[64];
		   snprintf(tempVar, 63, "xlat_funcalltemp%d", goit->swizzleAssignTempCounter ++);
		   goit->tempVariableName = tempVar;
		   delete[] tempVar;
		   TType *type = node->getTypePointer();
		   current->beginStatement();
		   writeTempVarAssign(type, goit->tempVariableName, goit, out);
	   }

	   if ( bGenMatrix )
	   {
		  if ( node->isMatrix () )
		  {
			 out << "xll_";
			 current->addLibFunction ( node->getOp() );
		  }
	   }      

	   out << name << "( ";

	   int paramIndex = 0;
		for (sit = sequence.begin(); sit != sequence.end(); ++sit, ++ paramIndex)
		{
			if (sit !=sequence.begin())
				out << ", ";
			out << paramsOut[paramIndex];
		}
		
	   out << ")";
	   if (needTempVar)
			current->endStatement();
   }
   goit->parentRequireValue = l_parentRequireValue;
   goit->isInlining = l_parentInlining;
}


void setupUnaryBuiltInFuncCall( const TString &name, TIntermUnary *node, TString &opStr, bool &funcStyle, bool &prefix,
                                TGlslOutputTraverser* goit )
{
   GlslFunction *current = goit->current;   

   funcStyle = true;
   prefix = true;
   if ( node->isMatrix() )
   {
      current->addLibFunction( node->getOp() );
      opStr = "xll_" + name;
   }
   else
   {
      opStr = name;
   }   
}



void writeTex( const TString &name, TIntermAggregate *node, TGlslOutputTraverser* goit )
{
	TIntermSequence &sequence = node->getSequence(); 
	TBasicType sampler_type = (*sequence.begin())->getAsTyped()->getBasicType();
	TString new_name;
	
	if (isShadowSampler(sampler_type)) {
		if (name == "texture2D")
			new_name = "shadow2D";
		else if (name == "texture2DProj")
			new_name = "shadow2DProj";
		else if (name == "texture1D")
			new_name = "shadow1D";
		else if (name == "texture1DProj")
			new_name = "shadow1DProj";
		else if (name == "texture2DRect")
			new_name = "shadow2DRect";
		else if (name == "texture2DRectProj")
			new_name = "shadow2DRectProj";
		else
			new_name = name;
	} else {
		new_name = name;
	}
	
	writeFuncCall(new_name, node, goit);
}

static bool SafeEquals(const char* a, const char* b)
{
    if((!a && b) || (a && !b))
    {
        return(false);
    }

    if(!a && !b)
    {
        return(true);
    }

    return(strcmp(a, b) == 0);
}

void TGlslOutputTraverser::outputLineDirective (const TSourceLoc& line)
{
	if (line.line <= 0 || !current)
		return;
	if (SafeEquals(line.file, m_LastLineOutput.file) && std::abs(line.line - m_LastLineOutput.line) < 4) // don't sprinkle too many #line directives ;)
		return;
	std::stringstream& out = current->getActiveOutput();
	out << '\n';
	current->indent(); // without this we could dry the code out further to put the preceeding CRLF in the shared function
	OutputLineDirective(out, line);
	m_LastLineOutput = line;
}



TGlslOutputTraverser::TGlslOutputTraverser(TInfoSink& i, 
										   std::vector<GlslFunction*> &funcList, 
										   std::vector<GlslStruct*> &sList, 
										   std::map<TString, TIntermAggregate*> &_inlinefuncList,
										   std::stringstream& deferredArrayInit, 
										   ETargetVersion version, 
										   unsigned options)
: infoSink(i)
, generatingCode(true)
, functionList(funcList)
, structList(sList)
, inlinefuncList(_inlinefuncList)
, m_DeferredArrayInit(deferredArrayInit)
, swizzleAssignTempCounter(0)
, m_TargetVersion(version)
, m_UsePrecision(Hlsl2Glsl_VersionUsesPrecision(version))
, m_ArrayInitWorkaround(options & ETranslateOpEmitGLSL120ArrayInitWorkaround)
, parentRequireValue(false)
, isInlining(false)
, inlineRetVar(NULL)
, inlineParamsMap(NULL)
, firstVisitDeclaration(false)
, declareUniform(false)
{
	m_LastLineOutput.file = NULL;
	m_LastLineOutput.line = -1;
	visitSymbol = traverseSymbol;
	visitConstant = traverseConstant;
	visitBinary = traverseBinary;
	visitUnary = traverseUnary;
	visitSelection = traverseSelection;
	visitAggregate = traverseAggregate;
	visitLoop = traverseLoop;
	visitBranch = traverseBranch;
	visitDeclaration = traverseDeclaration;
	
	TSourceLoc oneSourceLoc;
	oneSourceLoc.file=NULL;
	oneSourceLoc.line=1;

	// Add a fake "global" function for declarations & initializers happening
	// at global scope.
	global = new GlslFunction( "__global__", "__global__", EgstVoid, EbpUndefined, "", oneSourceLoc);
	functionList.push_back(global);
	current = global;
}



void TGlslOutputTraverser::traverseArrayDeclarationWithInit(TIntermDeclaration* decl)
{
	assert(decl->containsArrayInitialization());
	
	std::stringstream* out = &current->getActiveOutput();
	TType& type = *decl->getTypePointer();
	EGlslSymbolType symbol_type = translateType(decl->getTypePointer());
	
	const bool emit_120_arrays = (m_TargetVersion >= ETargetGLSL_120);
	const bool emit_old_arrays = !emit_120_arrays || m_ArrayInitWorkaround;
	const bool emit_both = emit_120_arrays && emit_old_arrays;
	
	if (emit_both)
	{
		current->indent(*out);
		(*out) << "#if defined(HLSL2GLSL_ENABLE_ARRAY_120_WORKAROUND)" << std::endl;
		current->increaseDepth();
	}
	
	if (emit_old_arrays)
	{
		assert(decl->isSingleInitialization() && "Emission of multiple in-line array declarations isn't supported when running in pre-GLSL1.20 mode.");
		
		TQualifier q = type.getQualifier();
		if (q == EvqConst)
			q = EvqTemporary;
		
		current->beginStatement();
		if (q != EvqTemporary && q != EvqGlobal)
			(*out) << type.getQualifierString() << " ";
		
		TIntermBinary* assign = decl->getDeclaration()->getAsBinaryNode();
		TIntermSymbol* sym = assign->getLeft()->getAsSymbolNode();
		TIntermSequence& init = assign->getRight()->getAsAggregate()->getSequence();
		
		writeType(*out, symbol_type, NULL, this->m_UsePrecision ? decl->getPrecision() : EbpUndefined);
		(*out) << " " << sym->getSymbol() << "[" << type.getArraySize() << "]";
		current->endStatement();

		std::stringstream* oldOut = out;
		if (sym->isGlobal())
		{
			current->pushDepth(0);
			out = &m_DeferredArrayInit;
			current->setActiveOutput(out);
		}
		
		unsigned n_vals = init.size();
		for (unsigned i = 0; i != n_vals; ++i) {
			current->beginStatement();
			sym->traverse(this);
			(*out) << "[" << i << "] = ";
			init[i]->traverse(this);
			current->endStatement();
		}
		
		if (sym->isGlobal())
		{
			out = oldOut;
			current->setActiveOutput(oldOut);
			current->popDepth();
		}
	}
	
	if (emit_both)
	{
		current->decreaseDepth();
		current->indent(*out);
		(*out) << "#else" << std::endl;
		current->increaseDepth();
	}
	
	if (emit_120_arrays)
	{	
		current->beginStatement();
		
		if (type.getQualifier() != EvqTemporary && type.getQualifier() != EvqGlobal)
			(*out) << type.getQualifierString() << " ";
		
		if (type.getBasicType() == EbtStruct)
			(*out) << type.getTypeName();
		else
			writeType(*out, symbol_type, NULL, this->m_UsePrecision ? decl->getPrecision() : EbpUndefined);
		
		if (type.isArray())
			(*out) << "[" << type.getArraySize() << "]";
		
		(*out) << " ";
		
		decl->getDeclaration()->traverse(this);
		
		current->endStatement();
	}
	
	if (emit_both)
	{
		current->decreaseDepth();
		current->indent(*out);
		(*out) << "#endif" << std::endl;
	}
}



bool TGlslOutputTraverser::traverseDeclaration(bool preVisit, TIntermDeclaration* decl, TIntermTraverser* it) {
	TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
	GlslFunction *current = goit->current;
	std::stringstream& out = current->getActiveOutput();
	bool l_parentIsVisitDeclaration = goit->firstVisitDeclaration;
	bool l_parentDeclareUniform = goit->declareUniform;

	if (decl->containsArrayInitialization())
	{
		goit->traverseArrayDeclarationWithInit (decl);
		return false;
	}

	//-------------------
	TType type = *decl->getTypePointer();
	goit->declareUniform = type.getQualifier() == EvqUniform;
	if (type.isNonSquareMatrix() && goit->declareUniform)//transform non-square matrix to array of vector
	{
		type.setBasicType(EbtFloat);
		type.setNominalSize(type.getNonSquareRows());
		if (!type.isArray())
			type.setArraySize(type.getNonSquareColumns());
		else
			type.setArraySize(type.getArraySize() * type.getNonSquareColumns());
		type.setArray(true);
	}

	goit->firstVisitDeclaration = true;

	//break multideclaration to multiple declaration statements
	TIntermSequence singleSeg;
	if (decl->isSingleDeclaration())
		singleSeg.push_back(decl->getDeclaration());

	TIntermSequence& declSeg = decl->isSingleDeclaration()? singleSeg: decl->getDeclaration()->getAsAggregate()->getSequence();

	for (TIntermSequence::iterator ite = declSeg.begin();
		ite != declSeg.end();
		++ite)
	{
#if defined DEBUG || defined _DEBUG
		TIntermSymbol * itermNode = dynamic_cast<TIntermSymbol*> (*ite);
#endif
		//visit initializer first
		std::stringstream initilizerOut;
		goit->tempVariableName = "";
		current->setActiveOutput(&initilizerOut);
		(*ite)->traverse(goit);

		//------------------------------
		current->setActiveOutput(&out);
		if (goit->tempVariableName.size() > 0)
		{
			current->beginStatement();
			out << initilizerOut.str();//write temp variable calculation first

			initilizerOut.str("");
			//assign being declared varible to this temp var
			current->setActiveOutput(&initilizerOut);

			(*ite)->getAsBinaryNode()->getLeft()->traverse(goit);//get variable name
			initilizerOut << " = " << goit->tempVariableName;

			current->setActiveOutput(&out);
		}

		current->beginStatement();
		
		if (type.getQualifier() != EvqTemporary && type.getQualifier() != EvqGlobal)
			out << type.getQualifierString() << " ";
		
		if (type.getBasicType() == EbtStruct)
		{
			out << type.getTypeName();
		}
		else
		{
			EGlslSymbolType symbol_type = translateType(&type);
			writeType(out, symbol_type, NULL, goit->m_UsePrecision ? decl->getPrecision() : EbpUndefined);
		}

		out << " ";
		
		out << initilizerOut.str();

		if (type.isArray())
			out << "[" << type.getArraySize() << "]";
		current->endStatement();
	}

	goit->firstVisitDeclaration = l_parentIsVisitDeclaration;
	goit->declareUniform = l_parentDeclareUniform;
	return false;
}


void TGlslOutputTraverser::traverseSymbol(TIntermSymbol *node, TIntermTraverser *it)
{
	TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
	GlslFunction *current = goit->current;
	std::stringstream& out = current->getActiveOutput();

	current->beginStatement();

	if ( ! current->hasSymbol( node->getId()))
	{

		//check to see if it is a global we can share
		if ( goit->global->hasSymbol( node->getId()))
		{
			current->addSymbol( &goit->global->getSymbol( node->getId()));
		}
		else
		{
			int array = node->getTypePointer()->isArray() ? node->getTypePointer()->getArraySize() : 0;
			const char* semantic = "";
			if (node->getInfo())
				semantic = node->getInfo()->getSemantic().c_str();
			
			GlslSymbol * sym = new GlslSymbol( node->getSymbol().c_str(), semantic, node->getId(),
				translateType(node->getTypePointer()), goit->m_UsePrecision?node->getPrecision():EbpUndefined, translateQualifier(node->getQualifier()), array);
			sym->setIsGlobal(node->isGlobal());

			current->addSymbol(sym);
			if (sym->getType() == EgstStruct)
			{
				GlslStruct *s = goit->createStructFromType( node->getTypePointer());
				sym->setStruct(s);
			}
		}
	}

	// If we're at the global scope, emit the non-mutable names of uniforms.
	bool globalScope = current == goit->global;
	bool inlineReplace = false;
	if (goit->isInlining)//inline expanding, replace parameter with temp var
	{
		std::map<int, std::string>::iterator ite = goit->inlineParamsMap->find(node->getId());
		if (ite != goit->inlineParamsMap->end())
		{
			out << ite->second;
			inlineReplace = true;
		}
	}
	if (!inlineReplace)
	{
		GlslSymbol &sym = current->getSymbol(node->getId());
		if (sym.getQualifier() == EqtUniform && sym.isNonSquareMatrix())//non-square matrix uniform will be transformed to array of vector
		{
			if (!goit->declareUniform)
				writeUniformNQMOperand(&sym, out);
			else
				out << sym.getName(!globalScope);
		}
		else
			out << sym.getName(!globalScope);
	}
}


void TGlslOutputTraverser::traverseParameterSymbol(TIntermSymbol *node, TIntermTraverser *it)
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;

   int array = node->getTypePointer()->isArray() ? node->getTypePointer()->getArraySize() : 0;
   const char* semantic = "";
   if (node->getInfo())
      semantic = node->getInfo()->getSemantic().c_str();
   GlslSymbol * sym = new GlslSymbol( node->getSymbol().c_str(), semantic, node->getId(),
                                      translateType(node->getTypePointer()), goit->m_UsePrecision?node->getPrecision():EbpUndefined, translateQualifier(node->getQualifier()), array);
   current->addParameter(sym);

   if (sym->getType() == EgstStruct)
   {
      GlslStruct *s = goit->createStructFromType( node->getTypePointer());
      sym->setStruct(s);
   }
}


void TGlslOutputTraverser::traverseConstant( TIntermConstant *node, TIntermTraverser *it )
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();
   EGlslSymbolType type = translateType( node->getTypePointer());
   GlslStruct *str = 0;


   current->beginStatement();

   if (type == EgstStruct)
   {
      str = goit->createStructFromType( node->getTypePointer());
   }

   writeConstantConstructor (out, type, goit->m_UsePrecision?node->getPrecision():EbpUndefined, node, str);
}


void TGlslOutputTraverser::traverseImmediateConstant( TIntermConstant *c, TIntermTraverser *it )
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);

   // These are all expected to be length 1
   assert(c->getSize() == 1);

   // Autotype the result
   switch (c->getBasicType())
   {
   case EbtBool:
      goit->indexList.push_back(c->toBool() ? 1 : 0);
      break;
   case EbtInt:
      goit->indexList.push_back(c->toInt());
      break;
   case EbtFloat:
      goit->indexList.push_back((int)c->toFloat());
      break;
   default:
      assert(false && "Invalid constant type. Only bool, int and float supported"); 
      goit->indexList.push_back(0);
   }
}


// Special case for matrix[idx1][idx2]: output as matrix[idx2][idx1]
static bool Check2DMatrixIndex (TGlslOutputTraverser* goit, std::stringstream& out, TIntermTyped *left, const std::string& rightExpr, bool needTempVarForRightIndex)
{
	GlslFunction *current =  goit->current;
	bool l_parentRequireValue = goit->parentRequireValue;

	if (left->isVector() && !left->isArray())
	{
		TIntermBinary* leftBin = left->getAsBinaryNode();
		if (leftBin && (leftBin->getOp() == EOpIndexDirect || leftBin->getOp() == EOpIndexIndirect))
		{
			TIntermTyped* superLeft = leftBin->getLeft();
			TIntermTyped* superRight = leftBin->getRight();
			if (superLeft->isMatrix() && !superLeft->isArray())
			{
				  bool needTempVar = needTempVarForRightIndex;
				  std::string tempVar;
				  std::stringstream superLeftOut;
				  std::stringstream superRightOut;
				//traverse superleft
				  if (superLeft)
				  {
						goit->parentRequireValue = true;//require it returns a value
						goit->tempVariableName = "";
						current->setActiveOutput(&superLeftOut);
						superLeft->traverse(goit);			
						goit->parentRequireValue = l_parentRequireValue;

						//-------------------
						current->setActiveOutput(&out);
						if (goit->tempVariableName.size() > 0)
						{
							out << superLeftOut.str();//write temporary variable calculation first
							current->endStatement();
							current->beginStatement();
							//use this temporary variable as left operand
							superLeftOut.str(goit->tempVariableName);

							needTempVar = needTempVar || l_parentRequireValue;
							
						}//if (goit->tempVariableName.size() > 0)
				  }
				  //traverse superRight
				  if (superRight)
				  {
						goit->parentRequireValue = true;//require it returns a value
						goit->tempVariableName = "";
						current->setActiveOutput(&superRightOut);
						superRight->traverse(goit);			
						goit->parentRequireValue = l_parentRequireValue;

						//-------------------
						current->setActiveOutput(&out);
						if (goit->tempVariableName.size() > 0)
						{
							out << superRightOut.str();//write temporary variable calculation first
							current->endStatement();
							current->beginStatement();
							//use this temporary variable as right operand
							superRightOut.str(goit->tempVariableName);

							needTempVar = needTempVar || l_parentRequireValue;
							
						}//if (goit->tempVariableName.size() > 0)
				  }
				  
				  if (needTempVar)
				  {
					char *tempVarBuffer = new char[64];
					snprintf(tempVarBuffer, 64, "xlat_binTemp%d", goit->swizzleAssignTempCounter ++);
					tempVar = tempVarBuffer;
					delete[] tempVarBuffer;

					  out << "float " << tempVar << " = ";
				  }

				out << superLeftOut.str();
				out << "[";
				out << rightExpr;
				out << "][";
				out << superRightOut.str();
				out << "]";
				
				goit->tempVariableName = tempVar;

				if (needTempVar)
					current->endStatement();

				return true;
			}
		}
	}
	return false;
}

bool TGlslOutputTraverser::traverseBinary( bool preVisit, TIntermBinary *node, TIntermTraverser *it )
{
   TString op = "??";
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();
   bool l_parentRequireValue = goit->parentRequireValue;
   bool infix = true;
   bool assign = false;
   bool needsParens = true;

   switch (node->getOp())
   {
   case EOpAssign:                   op = "=";   infix = true; needsParens = false; break;
   case EOpAddAssign:                op = "+=";  infix = true; needsParens = false; break;
   case EOpSubAssign:                op = "-=";  infix = true; needsParens = false; break;
   case EOpMulAssign:                op = "*=";  infix = true; needsParens = false; break;
   case EOpVectorTimesMatrixAssign:  op = "*=";  infix = true; needsParens = false; break;
   case EOpVectorTimesScalarAssign:  op = "*=";  infix = true; needsParens = false; break;
   case EOpMatrixTimesScalarAssign:  op = "*=";  infix = true; needsParens = false; break;
   case EOpMatrixTimesMatrixAssign:  op = "matrixCompMult";  infix = false; assign = true; break;
   case EOpDivAssign:                op = "/=";  infix = true; needsParens = false; break;
   case EOpModAssign:                op = "%=";  infix = true; needsParens = false; break;
   case EOpAndAssign:                op = "&=";  infix = true; needsParens = false; break;
   case EOpInclusiveOrAssign:        op = "|=";  infix = true; needsParens = false; break;
   case EOpExclusiveOrAssign:        op = "^=";  infix = true; needsParens = false; break;
   case EOpLeftShiftAssign:          op = "<<="; infix = true; needsParens = false; break;
   case EOpRightShiftAssign:         op = "??="; infix = true; needsParens = false; break;

   case EOpIndexDirect:
      {
		TIntermTyped *left = node->getLeft();
		TIntermTyped *right = node->getRight();
		assert( left && right);

		std::stringstream rvalOut;
		std::stringstream lvalOut;
		std::string tempVar;
		bool nonSquareUniform = (left!= NULL) && left->isNonSquareMatrix() && left->getQualifier() == EvqUniform;
		bool needTempVar = false;
		//traverse right operand
		if (right)
		{
			goit->parentRequireValue = true;//require it returns a value
			goit->tempVariableName = "";
			current->setActiveOutput(&rvalOut);
			right->traverse(goit);			
			goit->parentRequireValue = l_parentRequireValue;

			//-------------------
			current->setActiveOutput(&out);
			if (goit->tempVariableName.size() > 0)
			{
				current->beginStatement();
				out << rvalOut.str();//write temporary variable calculation first
				current->endStatement();
				//use this temporary variable as index
				rvalOut.str(goit->tempVariableName);

				needTempVar = l_parentRequireValue;
			}

		}

		current->beginStatement();

		if (Check2DMatrixIndex (goit, out, left, rvalOut.str(), needTempVar))
		  return false;

		 //traverse left
		  if (left)
		  {
			  if (nonSquareUniform)
			  {
				  lvalOut.str(left->getAsSymbolNode()->getSymbol().c_str());//use the name of uniform as left operand
			  }
			  else
			  {
				goit->parentRequireValue = true;//require it returns a value
				goit->tempVariableName = "";
				current->setActiveOutput(&lvalOut);
				left->traverse(goit);			
				goit->parentRequireValue = l_parentRequireValue;

				//-------------------
				current->setActiveOutput(&out);
				if (goit->tempVariableName.size() > 0)
				{
					out << lvalOut.str();//write temporary variable calculation first
					current->endStatement();
					current->beginStatement();
					//use this temporary variable as left operand
					lvalOut.str(goit->tempVariableName);

					needTempVar = needTempVar || l_parentRequireValue;
					
				}//if (goit->tempVariableName.size() > 0)
			  }
		  }
		  
		  if (needTempVar)
		  {
			char *tempVarBuffer = new char[64];
			snprintf(tempVarBuffer, 64, "xlat_binTemp%d", goit->swizzleAssignTempCounter ++);
			tempVar = tempVarBuffer;
			delete[] tempVarBuffer;

			  writeTempVarAssign(node->getTypePointer(), tempVar, goit, out);
		  }

		 if (!left->isArray())
		 {
			 if (left->isMatrix())
			 {
#if 1//need to use transpose version of square matrix, since GLSL matrix operation is in reverse order compare to HLSL
				 if (right->getAsConstant())
				 {
					 current->addLibFunction (EOpMatrixIndex);
					 out << "xll_matrixindex (";
					 out << lvalOut.str();
					 out << ", ";
					 out << rvalOut.str();
					 out << ")";

					 goit->tempVariableName = tempVar;
					  if (needTempVar)
						  current->endStatement();
					 return false;
				 }
				 else
				 {
					 current->addLibFunction (EOpTranspose);
					 current->addLibFunction (EOpMatrixIndex);
					 current->addLibFunction (EOpMatrixIndexDynamic);
					 out << "xll_matrixindexdynamic (";
					 out << lvalOut.str();
					 out << ", ";
					 out << rvalOut.str();
					 out << ")";

					 goit->tempVariableName = tempVar;
					  if (needTempVar)
						  current->endStatement();

					 return false;
				 }
#endif
			 }//if (left->isMatrix())
		 }//if (!left->isArray())

		if (nonSquareUniform)//uniform non-square matrix is transformed to array, so it uses a different way
		{
			writeUniformNQMArrayElementOperand(left->getTypePointer(), left->getAsSymbolNode()->getSymbol().c_str(), rvalOut.str().c_str(), out);
		}
		else
		{
			 out << lvalOut.str();

			 // Special code for handling a vector component select (this improves readability)
			 if (left->isVector() && !left->isArray() && right->getAsConstant())
			 {
				char swiz[] = "xyzw";
				goit->visitConstant = TGlslOutputTraverser::traverseImmediateConstant;
				goit->generatingCode = false;
				right->traverse(goit);
				assert( goit->indexList.size() == 1);
				assert( goit->indexList[0] < 4);
				out << "." << swiz[goit->indexList[0]];
				goit->indexList.clear();
				goit->visitConstant = TGlslOutputTraverser::traverseConstant;
				goit->generatingCode = true;
			 }
			 else
			 {
				out << "[";
				out << rvalOut.str();
				out << "]";
			 }
		}
		  goit->tempVariableName = tempVar;
		  if (needTempVar)
			  current->endStatement();
         return false;
      }
   case EOpIndexIndirect:
      {
      TIntermTyped *left = node->getLeft();
      TIntermTyped *right = node->getRight();

	   std::stringstream lvalOut;
	   std::stringstream rvalOut;
	   std::string tempVar;
	   bool nonSquareUniform = (left!= NULL) && left->isNonSquareMatrix() && left->getQualifier() == EvqUniform;
	   bool needTempVar = false;
	   //traverse right operand
	   if (right)
	   {
			goit->parentRequireValue = true;//require it returns a value
			goit->tempVariableName = "";
			current->setActiveOutput(&rvalOut);
			right->traverse(goit);			
			goit->parentRequireValue = l_parentRequireValue;

			//-------------------
			current->setActiveOutput(&out);
			if (goit->tempVariableName.size() > 0)
			{
				current->beginStatement();
				out << rvalOut.str();//write temporary variable calculation first
				current->endStatement();
				//use this temporary variable as index
				rvalOut.str(goit->tempVariableName);

				needTempVar = l_parentRequireValue;
			}
			else if (nonSquareUniform)//need to create a temporary var holding index value
			{
				char *tempVarBuffer = new char[64];
				snprintf(tempVarBuffer, 64, "xlat_indexTemp%d", goit->swizzleAssignTempCounter ++);
				
				//assign rval to temporary index var
				current->beginStatement();
				out << "int " << tempVarBuffer << " = ";
				out << rvalOut.str();
				current->endStatement();

				rvalOut.str(tempVarBuffer);//use this temp var as index expression

				delete[] tempVarBuffer;

				needTempVar = l_parentRequireValue;

			}

	   }
	  
      current->beginStatement();

	  if (Check2DMatrixIndex (goit, out, left, rvalOut.str(), needTempVar))
		  return false;
	
	  
	  //traverse left
	  if (left)
	  {
		  if (nonSquareUniform)
		  {
			  lvalOut.str(left->getAsSymbolNode()->getSymbol().c_str());//use the name of uniform as left operand
		  }
		  else
		  {
			goit->parentRequireValue = true;//require it returns a value
			goit->tempVariableName = "";
			current->setActiveOutput(&lvalOut);
			left->traverse(goit);			
			goit->parentRequireValue = l_parentRequireValue;

			//-------------------
			current->setActiveOutput(&out);
			if (goit->tempVariableName.size() > 0)
			{
				out << lvalOut.str();//write temporary variable calculation first
				current->endStatement();
				current->beginStatement();
				//use this temporary variable as left operand
				lvalOut.str(goit->tempVariableName);

				needTempVar = needTempVar || l_parentRequireValue;
				
			}//if (goit->tempVariableName.size() > 0)
		  }
	  }
	  
	  if (needTempVar)
	  {
		char *tempVarBuffer = new char[64];
		snprintf(tempVarBuffer, 64, "xlat_binTemp%d", goit->swizzleAssignTempCounter ++);
		tempVar = tempVarBuffer;
		delete[] tempVarBuffer;

		  writeTempVarAssign(node->getTypePointer(), tempVar, goit, out);
	  }
	  
	  if (left && right && left->isMatrix() && !left->isArray())
	  {
		  if (right->getAsConstant())
		  {
			  current->addLibFunction (EOpMatrixIndex);
			  out << "xll_matrixindex (";
			  out << lvalOut.str();
			  out << ", ";
			  out << rvalOut.str();
			  out << ")";

			  goit->tempVariableName = tempVar;
			  if (needTempVar)
				  current->endStatement();

			  return false;
		  }
		  else
		  {
			  current->addLibFunction (EOpTranspose);
			  current->addLibFunction (EOpMatrixIndex);
			  current->addLibFunction (EOpMatrixIndexDynamic);
			  out << "xll_matrixindexdynamic (";
			  out << lvalOut.str();
			  out << ", ";
			  out << rvalOut.str();
			  out << ")";

			  goit->tempVariableName = tempVar;
			  if (needTempVar)
				  current->endStatement();

			  return false;
		  }
	  }
	  

	  if (nonSquareUniform)//uniform non-square matrix is transformed to array, so it uses a different way
	  {
		  writeUniformNQMArrayElementOperand(left->getTypePointer(), left->getAsSymbolNode()->getSymbol().c_str(), rvalOut.str().c_str(), out);
	  }
	  else
	  {
		  if (left)
			 out << lvalOut.str();
		  out << "[";
		  if (right)
			out << rvalOut.str();
		  out << "]";
	  }

	  goit->tempVariableName = tempVar;
	  if (needTempVar)
		  current->endStatement();

      return false;
	  }

   case EOpIndexDirectStruct:
      {
		 std::stringstream indexPrefix;
		 std::stringstream leftOut;
		 bool isUniformNonSquareMatrix = false;
		 std::string tempVar;
         GlslStruct *s = goit->createStructFromType(node->getLeft()->getTypePointer());
         if (node->getLeft())
		 {
			 TIntermTyped *left = node->getLeft();
			 if (left->getAsBinaryNode())
			 {
				 TIntermBinary *binNode = left->getAsBinaryNode();
				 if (binNode->getLeft() != NULL && binNode->getLeft()->isNonSquareMatrix() && binNode->getLeft()->getQualifier() == EvqUniform)
				 {
					 TIntermTyped *leftOfBin = binNode->getLeft();
					 TIntermTyped *rightOfBin = binNode->getRight();
					 isUniformNonSquareMatrix = true;
					 std::stringstream rvalOut;
					 //traverse right index expression operand
					 if (rightOfBin)
					 {
						goit->parentRequireValue = true;//require it returns a value
						goit->tempVariableName = "";
						current->setActiveOutput(&rvalOut);
						rightOfBin->traverse(goit);			
						goit->parentRequireValue = l_parentRequireValue;

						//-------------------
						current->setActiveOutput(&out);
						if (goit->tempVariableName.size() > 0)
						{
							current->beginStatement();
							out << rvalOut.str();//write temporary variable calculation first
							current->endStatement();
							//use this temporary variable as index
							rvalOut.str(goit->tempVariableName);

							if (l_parentRequireValue)
							{
								char *tempVarBuffer = new char[64];
								snprintf(tempVarBuffer, 64, "xlat_binTemp%d", goit->swizzleAssignTempCounter ++);
								tempVar = tempVarBuffer;
								delete[] tempVarBuffer;

							}
							
						}//if (goit->tempVariableName.size() > 0)

					 }//if (rightOfBin)
					 //get prefix index
					 indexPrefix << leftOfBin->getTypePointer()->getNonSquareColumns();
					 indexPrefix << " * (";
					 indexPrefix << rvalOut.str();
					 indexPrefix << ") + ";
					
					 leftOut.str(leftOfBin->getAsSymbolNode()->getSymbol().c_str());

				 }//if (binNode->getLeft() != NULL && binNode->getLeft()->isNonSquareMatrix() && binNode->getLeft()->getQualifier() == EvqUniform)
			 }//if (left->getAsBinaryNode())
			 else if (left->isNonSquareMatrix() && left->getQualifier() == EvqUniform)
			 {
				 isUniformNonSquareMatrix = true;
				 leftOut.str(left->getAsSymbolNode()->getSymbol().c_str());
			 }//else if (left->isNonSquareMatrix() && left->getQualifier() == EvqUniform)
			 
			 if (!isUniformNonSquareMatrix)
			 {
				goit->parentRequireValue = true;//require it returns a value
				goit->tempVariableName = "";
				current->setActiveOutput(&leftOut);
				left->traverse(goit);			
				goit->parentRequireValue = l_parentRequireValue;

				//-------------------
				current->setActiveOutput(&out);
				if (goit->tempVariableName.size() > 0)
				{
					current->beginStatement();
					out << leftOut.str();//write temporary variable calculation first
					current->endStatement();
					//use this temporary variable as left operand
					leftOut.str(goit->tempVariableName);

					if (l_parentRequireValue)
					{
						char *tempVarBuffer = new char[64];
						snprintf(tempVarBuffer, 64, "xlat_binTemp%d", goit->swizzleAssignTempCounter ++);
						tempVar = tempVarBuffer;
						delete[] tempVarBuffer;

					}
					
				}//if (goit->tempVariableName.size() > 0)
			 }//if (!isUniformNonSquareMatrix)
		 }
         current->beginStatement();
		 if (tempVar.size() > 0)
			 writeTempVarAssign(node->getTypePointer(), tempVar, goit, out);

		 out << leftOut.str();
         // The right child is always an offset into the struct, switch to get an
         // immediate constant, and put it back afterwords
         goit->visitConstant = TGlslOutputTraverser::traverseImmediateConstant;
         goit->generatingCode = false;

         if (node->getRight())
         {
            node->getRight()->traverse(goit);
            assert( goit->indexList.size() == 1);
            assert( goit->indexList[0] < s->memberCount());
			if (isUniformNonSquareMatrix)//use diffrent way for non-square matrix uniform, because it is transformed to array of vector
			{
				out << "[";
				out << indexPrefix.str() << goit->indexList[0];
				out << "]";
			}
			else
				out << "." << s->getMember(goit->indexList[0]).name;

         }

         goit->indexList.clear();
         goit->visitConstant = TGlslOutputTraverser::traverseConstant;
         goit->generatingCode = true;
		 goit->tempVariableName = tempVar;
		 if (tempVar.size() > 0)
			 current->endStatement();
      }
      return false;

   case EOpVectorSwizzle:
      current->beginStatement();
      if (node->getLeft())
         node->getLeft()->traverse(goit);
      goit->visitConstant = TGlslOutputTraverser::traverseImmediateConstant;
      goit->generatingCode = false;
      if (node->getRight())
      {
         node->getRight()->traverse(goit);
         assert( goit->indexList.size() <= 4);
         out << '.';
         const char fields[] = "xyzw";
         for (int ii = 0; ii < (int)goit->indexList.size(); ii++)
         {
            int val = goit->indexList[ii];
            assert( val >= 0);
            assert( val < 4);
            out << fields[val];
         }
      }
      goit->indexList.clear();
      goit->visitConstant = TGlslOutputTraverser::traverseConstant;
      goit->generatingCode = true;
      return false;

	case EOpMatrixSwizzle:		   
		// This presently only works for swizzles as rhs operators
		if (node->getRight())
		{
			goit->visitConstant = TGlslOutputTraverser::traverseImmediateConstant;
			goit->generatingCode = false;

			node->getRight()->traverse(goit);

			goit->visitConstant = TGlslOutputTraverser::traverseConstant;
			goit->generatingCode = true;

			std::vector<int> elements = goit->indexList;
			goit->indexList.clear();
			
			if (elements.size() > 4 || elements.size() < 1) {
				goit->infoSink.info << "Matrix swizzle operations can must contain at least 1 and at most 4 element selectors.";
				return true;
			}

			unsigned column[4] = {0}, row[4] = {0};
			for (unsigned i = 0; i != elements.size(); ++i)
			{
				unsigned val = elements[i];
				column[i] = val % 4;
				row[i] = val / 4;
			}

			bool sameColumn = true;
			for (unsigned i = 1; i != elements.size(); ++i)
				sameColumn &= column[i] == column[i-1];

			static const char* fields = "xyzw";
			
			if (sameColumn)
			{				
				//select column, then swizzle row
				if (node->getLeft())
					node->getLeft()->traverse(goit);
				out << "[" << column[0] << "].";
				
				for (unsigned i = 0; i < elements.size(); ++i)
					out << fields[row[i]];
			}
			else
			{
				// Insert constructor, and dereference individually

				// Might need to account for different types here 
				assert( elements.size() != 1); //should have hit same collumn case
				out << "vec" << elements.size() << "(";
				if (node->getLeft())
					node->getLeft()->traverse(goit);
				out << "[" << column[0] << "].";
				out << fields[row[0]];
				
				for (unsigned i = 1; i < elements.size(); ++i)
				{
					out << ", ";
					if (node->getLeft())
						node->getLeft()->traverse(goit);
					out << "[" << column[i] << "].";
					out << fields[row[i]];
				}
				out << ")";
			}
		}
		return false;

   case EOpAdd:    op = "+"; infix = true; break;
   case EOpSub:    op = "-"; infix = true; break;
   case EOpMul:    op = "*"; infix = true; break;
   case EOpDiv:    op = "/"; infix = true; break;
   case EOpMod:    op = "mod"; infix = false; break;
   case EOpRightShift:  op = "<<"; infix = true; break;
   case EOpLeftShift:   op = ">>"; infix = true; break;
   case EOpAnd:         op = "&"; infix = true; break;
   case EOpInclusiveOr: op = "|"; infix = true; break;
   case EOpExclusiveOr: op = "^"; infix = true; break;
   case EOpEqual:       
      writeComparison ( "==", "equal", node, goit );
      return false;        

   case EOpNotEqual:        
      writeComparison ( "!=", "notEqual", node, goit );
      return false;               

   case EOpLessThan: 
      writeComparison ( "<", "lessThan", node, goit );
      return false;               

   case EOpGreaterThan:
      writeComparison ( ">", "greaterThan", node, goit );
      return false;               

   case EOpLessThanEqual:    
      writeComparison ( "<=", "lessThanEqual", node, goit );
      return false;               


   case EOpGreaterThanEqual: 
      writeComparison ( ">=", "greaterThanEqual", node, goit );
      return false;               


   case EOpVectorTimesScalar: op = "*"; infix = true; break;
   case EOpVectorTimesMatrix: op = "*"; infix = true; break;
   case EOpMatrixTimesVector: op = "*"; infix = true; break;
   case EOpMatrixTimesScalar: op = "*"; infix = true; break;
   case EOpMatrixTimesMatrix: op = "matrixCompMult"; infix = false; assign = false; break;

   case EOpLogicalOr:  op = "||"; infix = true; break;
   case EOpLogicalXor: op = "^^"; infix = true; break;
   case EOpLogicalAnd: op = "&&"; infix = true; break;
   default: assert(0);
   }

   current->beginStatement();

   if (infix)
   {
	   // special case for swizzled matrix of matrix index subscripting assignment
	   if ((node->getOp() == EOpAssign || node->getOp() == EOpAddAssign 
		   || node->getOp() == EOpSubAssign || node->getOp() == EOpMulAssign 
		   || node->getOp() == EOpDivAssign  /*|| node->getOp() == EOpVectorTimesMatrixAssign*/
		   || node->getOp() == EOpVectorTimesScalarAssign || node->getOp() == EOpInclusiveOrAssign
		   || node->getOp() == EOpExclusiveOrAssign || node->getOp() == EOpLeftShiftAssign ||
		   node->getOp() == EOpRightShiftAssign)
		   && node->getLeft() && node->getRight()) {
		   TIntermBinary* lval = node->getLeft()->getAsBinaryNode();
		   
		   if (lval)
		   {
			   const char* _operator = op.c_str();
				

			   if( lval->getOp() == EOpMatrixSwizzle) 
			   {
				   static const char* vec_swizzles = "xyzw";
				   TIntermTyped* rval = node->getRight();
				   TIntermTyped* lexp = lval->getLeft();
				   
				   goit->visitConstant = TGlslOutputTraverser::traverseImmediateConstant;
				   goit->generatingCode = false;
				   
				   lval->getRight()->traverse(goit);
				   
				   goit->visitConstant = TGlslOutputTraverser::traverseConstant;
				   goit->generatingCode = true;
				   
				   std::vector<int> swizzles = goit->indexList;
				   goit->indexList.clear();
				   
				   char temp_rval[64];
				   unsigned n_swizzles = swizzles.size();
					bool tempRValVar = false;

				   std::stringstream rvalOut;

				   //traverse right operand
					goit->parentRequireValue = true;//require it returns a value
					goit->tempVariableName = "";
					current->setActiveOutput(&rvalOut);
					current->beginStatement();
					rval->traverse(goit);			   
					current->endStatement();

					//-------------------
					current->setActiveOutput(&out);
					if (goit->tempVariableName.size() > 0)
					{
						tempRValVar = true;
						out << rvalOut.str();//write temporary variable calculation first
						//use this temporary variable instead of new one
						strcpy(temp_rval, goit->tempVariableName.c_str());
					}
					else
					{
						if (n_swizzles > 1) {
							tempRValVar = true;
							//create new temporary variable
							snprintf(temp_rval, 64, "xlat_rvaltemp%d", goit->swizzleAssignTempCounter++);
							
							//store value in temp variable first
							current->beginStatement();
							if (rval->getNominalSize() > 1)
								out << "vec" << rval->getNominalSize() << " " << temp_rval << " = " << rvalOut.str();
							else
								out << "float " << temp_rval << " = " << rvalOut.str();
						}
					}

					if (l_parentRequireValue && n_swizzles > 1)
					{
						if (rval->getNominalSize() != lval->getNominalSize())//need another temp var
						{
							char *temp_retNameBuffer = new char[64];
							snprintf(temp_retNameBuffer, 64, "xlat_swizTemp%d", goit->swizzleAssignTempCounter++);
							goit->tempVariableName = temp_retNameBuffer;
							delete[] temp_retNameBuffer;
							
							//declare it
							current->beginStatement();
							out << "vec" << lval->getNominalSize() << " " << goit->tempVariableName ;	
							current->endStatement();
						}
						else
							goit->tempVariableName = temp_rval;
					}
					else
						goit->tempVariableName = "";
				

					for (unsigned i = 0; i != n_swizzles; ++i) {
					   unsigned col = swizzles[i] / 4;
					   unsigned row = swizzles[i] % 4;
					   
					   current->beginStatement();
					   if (l_parentRequireValue && n_swizzles > 1)
					   {
						   out << goit->tempVariableName << "." << vec_swizzles[i] << " = " ;
					   }
					   lexp->traverse(goit);
					   out << "[" << row << "][" << col << "] " << _operator << " ";
					   if (tempRValVar)
					   {
						   out << temp_rval;
						   if (rval->getNominalSize() > 1)
								out << "." << vec_swizzles[i];
					   }
					   else
						   out << rvalOut.str();
					   
					   current->endStatement();
					}

					goit->parentRequireValue = l_parentRequireValue;

					return false;
				}//if (lval && lval->getOp() == EOpMatrixSwizzle)
			   else if (((lval->getOp() == EOpIndexDirect) || lval->getOp() == EOpIndexIndirect )
				   && !lval->getLeft()->isArray() && lval->getLeft()->isMatrix() 
				   )
			   {
					//access row of matrix instead of column

				   TIntermTyped* rval = node->getRight();
				   TIntermTyped* lexp = lval->getLeft();
				   
				   char temp_rval[64];
				   char temp_indexval[64];

				   std::stringstream rvalOut;
					   
				   //traverse right operand
				   goit->parentRequireValue = true;//require it returns a value
				   goit->tempVariableName = "";
				   current->setActiveOutput(&rvalOut);
				   current->beginStatement();
					rval->traverse(goit);			   
					current->endStatement();
					
					//--------------------------
					current->setActiveOutput(&out);
					if (goit->tempVariableName.size() > 0)
					{
						out << rvalOut.str();//write temporary variable calculation first
						//use this temporary variable instead of new one
						strcpy(temp_rval, goit->tempVariableName.c_str());
					}
					else
					{
						//create new temporary variable
						snprintf(temp_rval, 64, "xlat_rvalTemp%d", goit->swizzleAssignTempCounter++);
						//store value in temp variable first
						current->beginStatement();
						if (rval->getNominalSize() > 1)//vector
							out << "vec" << rval->getNominalSize() << " " << temp_rval << " = " << rvalOut.str();
						else
						{
							out << "float " << temp_rval << " = " << rvalOut.str();
						}
					}
					
					if (l_parentRequireValue)
					{
						if (rval->getNominalSize() != lval->getNominalSize())//need another temp var
						{
							char *temp_retNameBuffer = new char[64];
							snprintf(temp_retNameBuffer, 64, "xlat_matRowTemp%d", goit->swizzleAssignTempCounter++);
							goit->tempVariableName = temp_retNameBuffer;
							delete[] temp_retNameBuffer;

							//declare it
							current->beginStatement();
							out << "vec" << lval->getNominalSize() << " " << goit->tempVariableName;
							current->endStatement();
						}
						else
							goit->tempVariableName = temp_rval;
					}
					else
						goit->tempVariableName = "";
	
					TIntermConstant *const_index = lval->getRight()->getAsConstant();
					//calculate index and store in temporary variable
					if (const_index == NULL)
					{
						snprintf(temp_indexval, 64, "xlat_indextemp%d", goit->swizzleAssignTempCounter++);
					
						current->beginStatement();
						if (lval->getRight()->getQualifier() == EvqConst)
							out << "const ";
						out << "int " << temp_indexval << " = ";
						lval->getRight()->traverse(goit);			   
						current->endStatement();
					}
					else
					{
						snprintf(temp_indexval, 64, "%d", const_index->toInt());
					}
				   
				   const char vec_swizzles[] = "xyzw";
				   for (unsigned i = 0; i != lval->getNominalSize(); ++i) {
					   current->beginStatement();
					   if (l_parentRequireValue)
					   {
						   out << goit->tempVariableName << "." << vec_swizzles[i] << " = " ;
					   }
					   lexp->traverse(goit);
					   out << "[" << i << "][" << temp_indexval << "] " << _operator << " ";
					   out << temp_rval;
					   if (rval->getNominalSize() == lval->getNominalSize())//is scalar or not?
							out << "." << vec_swizzles[i];
					   
					   current->endStatement();
				   }

					
				   goit->parentRequireValue = l_parentRequireValue;
				   return false;
			   }
		   }//if (lval)
	   }

	   std::string leftOut;
	   std::stringstream childOutStream;//this also be used as rightOut
	   bool needTempVar = false;
	   bool l_parentFirstVisitDeclaration = goit->firstVisitDeclaration;//save state
	   goit->firstVisitDeclaration = false;//avoid telling recursive traversal that we are first visiting declaration
	   //----traverse left and right-------------------
		goit->parentRequireValue = true;//require them return values
		//left
		goit->tempVariableName = "";
		current->setActiveOutput(&childOutStream);
		if (node->getLeft())
		{
#if defined DEBUG || defined _DEBUG
			TIntermDeclaration* declNode = node->getLeft()->getAsDeclaration();
#endif
			node->getLeft()->traverse(goit);	
			if (goit->tempVariableName.size() > 0)//we have temporary variable
			{
				out << childOutStream.str();//write its calculation first
				leftOut = goit->tempVariableName;
				needTempVar = true;
			}
			else
				leftOut = childOutStream.str();
		}
		//right
		goit->tempVariableName = "";
		childOutStream.str("");//clear right's output stream
		current->setActiveOutput(&childOutStream);
		if (node->getRight())
		{
			node->getRight()->traverse(goit);	
			if (goit->tempVariableName.size() > 0)//we have temporary variable
			{
				out << childOutStream.str();//write its calculation first
				childOutStream.str(goit->tempVariableName);
				needTempVar = true;
			}
		}
		//----------------------
		
		current->setActiveOutput(&out);
		goit->tempVariableName = "";
		needTempVar = needTempVar && (l_parentRequireValue || l_parentFirstVisitDeclaration);//really need temp var ?
		current->beginStatement();
		if (needTempVar)
		{
			char *tempVar = new char[64];
			snprintf(tempVar, 64, "xlat_bintemp%d", goit->swizzleAssignTempCounter++);
			goit->tempVariableName = tempVar;
			delete[] tempVar;

			writeTempVarAssign(node->getTypePointer(), goit->tempVariableName, goit, out);
		}

		if (needsParens)
		 out << '(';

		if (!l_parentFirstVisitDeclaration)
		{
			if (node->getLeft())
				out << leftOut;
			out << ' ' << op << ' ';
		}
		if (node->getRight())
			out << childOutStream.str();
		if (needsParens)
		 out << ')';

		if (needTempVar)
		  current->endStatement();

		//restore states
		goit->parentRequireValue = l_parentRequireValue;
		goit->firstVisitDeclaration = l_parentFirstVisitDeclaration; 
   }
   else
   {
	   std::string leftOut;
	   std::stringstream childOutStream;//this also be used as rightOut
	   bool needTempVar = false;
	   //----traverse left and right-------------------
		goit->parentRequireValue = true;//require them return values
		//left
		goit->tempVariableName = "";
		current->setActiveOutput(&childOutStream);
		if (node->getLeft())
		{
			node->getLeft()->traverse(goit);	
			if (goit->tempVariableName.size() > 0)//we have temporary variable
			{
				out << childOutStream.str();//write its calculation first
				leftOut = goit->tempVariableName;
				needTempVar = true;
			}
			else
				leftOut = childOutStream.str();
		}
		//right
		goit->tempVariableName = "";
		childOutStream.str("");//clear right's output stream
		current->setActiveOutput(&childOutStream);
		if (node->getRight())
		{
			node->getRight()->traverse(goit);	
			if (goit->tempVariableName.size() > 0)//we have temporary variable
			{
				out << childOutStream.str();//write its calculation first
				childOutStream.str(goit->tempVariableName);
				needTempVar = true;
			}
		}
		//-------------------------
		current->setActiveOutput(&out);
		if (assign)
		{		  
			// Need to traverse the left child twice to allow for the assign and the op
			// This is OK, because we know it is an lvalue
			if (node->getLeft())
				node->getLeft()->traverse(goit);

			out << " = " << op << '(';

			if (node->getLeft())
				out << leftOut;
			out << ", ";
			if (node->getRight())
				out << childOutStream.str();

			out << ')';
		}
		else
		{
			needTempVar = needTempVar && l_parentRequireValue;
			current->beginStatement();
			if (needTempVar)
			{
				char *tempVar = new char[64];
				snprintf(tempVar, 64, "xlat_bintemp%d", goit->swizzleAssignTempCounter++);
				goit->tempVariableName = tempVar;
				delete[] tempVar;

				writeTempVarAssign(node->getTypePointer(), goit->tempVariableName, goit, out);
			}

			out << op << '(';

			if (node->getLeft())
				out << leftOut;
			out << ", ";
			if (node->getRight())
				out << childOutStream.str();

			out << ')';
			if (needTempVar)
				current->endStatement();
		}

		goit->parentRequireValue = l_parentRequireValue;
   }

   return false;
}


bool TGlslOutputTraverser::traverseUnary( bool preVisit, TIntermUnary *node, TIntermTraverser *it )
{
   TString op("??");
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();
   bool l_parentRequireValue = goit->parentRequireValue;
   bool funcStyle = false;
   bool prefix = true;
   char zero[] = "0";

   current->beginStatement();

   switch (node->getOp())
   {
   case EOpNegative:       op = "-";  funcStyle = false; prefix = true; break;
   case EOpVectorLogicalNot:
   case EOpLogicalNot:     op = "!";  funcStyle = false; prefix = true; break;
   case EOpBitwiseNot:     op = "-";  funcStyle = false; prefix = true; break;

   case EOpPostIncrement:  op = "++"; funcStyle = false; prefix = false; break;
   case EOpPostDecrement:  op = "--"; funcStyle = false; prefix = false; break;
   case EOpPreIncrement:   op = "++"; funcStyle = false; prefix = true; break;
   case EOpPreDecrement:   op = "--"; funcStyle = false; prefix = true; break;

   case EOpConvIntToBool:
   case EOpConvFloatToBool:
      op = "bool";
      if ( node->getTypePointer()->getNominalSize() > 1)
      {
         zero[0] += node->getTypePointer()->getNominalSize();
         op = TString("bvec") + zero; 
      }
      funcStyle = true;
      prefix = true;
      break;

   case EOpConvBoolToFloat:
   case EOpConvIntToFloat:
      op = "float";
      if ( node->getTypePointer()->getNominalSize() > 1)
      {
         zero[0] += node->getTypePointer()->getNominalSize();
         op = TString("vec") + zero; 
      }
      funcStyle = true;
      prefix = true;
      break;

   case EOpConvFloatToInt: 
   case EOpConvBoolToInt:
      op = "int";
      if ( node->getTypePointer()->getNominalSize() > 1)
      {
         zero[0] += node->getTypePointer()->getNominalSize();
         op = TString("ivec") + zero; 
      }
      funcStyle = true;
      prefix = true;
      break;

   case EOpRadians:        setupUnaryBuiltInFuncCall ( "radians", node, op, funcStyle, prefix, goit );  break;
   case EOpDegrees:        setupUnaryBuiltInFuncCall ( "degrees", node, op, funcStyle, prefix, goit ); break;
   case EOpSin:            setupUnaryBuiltInFuncCall ( "sin", node, op, funcStyle, prefix, goit ); break;
   case EOpCos:            setupUnaryBuiltInFuncCall ( "cos", node, op, funcStyle, prefix, goit ); break;
   case EOpTan:            setupUnaryBuiltInFuncCall ( "tan", node, op, funcStyle, prefix, goit ); break;
   case EOpAsin:           setupUnaryBuiltInFuncCall ( "asin", node, op, funcStyle, prefix, goit ); break;
   case EOpAcos:           setupUnaryBuiltInFuncCall ( "acos", node, op, funcStyle, prefix, goit ); break;
   case EOpAtan:           setupUnaryBuiltInFuncCall ( "atan", node, op, funcStyle, prefix, goit ); break;
   
   case EOpExp:            setupUnaryBuiltInFuncCall ( "exp", node, op, funcStyle, prefix, goit ); break;
   case EOpLog:            setupUnaryBuiltInFuncCall ( "log", node, op, funcStyle, prefix, goit ); break;
   case EOpExp2:           setupUnaryBuiltInFuncCall ( "exp2", node, op, funcStyle, prefix, goit ); break;
   case EOpLog2:           setupUnaryBuiltInFuncCall ( "log2", node, op, funcStyle, prefix, goit ); break;
   case EOpSqrt:           setupUnaryBuiltInFuncCall ( "sqrt", node, op, funcStyle, prefix, goit ); break;
   case EOpInverseSqrt:    setupUnaryBuiltInFuncCall ( "inversesqrt", node, op, funcStyle, prefix, goit ); break;

   case EOpAbs:            setupUnaryBuiltInFuncCall ( "abs", node, op, funcStyle, prefix, goit ); break;
   case EOpSign:           setupUnaryBuiltInFuncCall ( "sign", node, op, funcStyle, prefix, goit ); break;
   case EOpFloor:          setupUnaryBuiltInFuncCall ( "floor", node, op, funcStyle, prefix, goit ); break;
   case EOpCeil:           setupUnaryBuiltInFuncCall ( "ceil", node, op, funcStyle, prefix, goit ); break;
   case EOpFract:          setupUnaryBuiltInFuncCall ( "fract", node, op, funcStyle, prefix, goit ); break;

   case EOpLength:         op = "length";  funcStyle = true; prefix = true; break;
   case EOpNormalize:      op = "normalize";  funcStyle = true; prefix = true; break;
   case EOpDPdx:           
	   current->addLibFunction(EOpDPdx);
	   op = "xll_dFdx";
	   funcStyle = true;
	   prefix = true;
	   break;
   case EOpDPdy:
	   current->addLibFunction(EOpDPdy);
	   op = "xll_dFdy";
	   funcStyle = true;
	   prefix = true;
	   break;
   case EOpFwidth:
	   current->addLibFunction(EOpFwidth);
	   op = "xll_fwidth";
	   funcStyle = true;
	   prefix = true;
	   break;
   case EOpFclip:		   
	  current->addLibFunction(EOpFclip);
      op = "xll_clip";
      funcStyle = true;
      prefix = true;
      break;    

	case EOpRound:
		current->addLibFunction(EOpRound);
		op = "xll_round";
		funcStyle = true;
		prefix = true;
		break;
	case EOpTrunc:
	   current->addLibFunction(EOpTrunc);
	   op = "xll_trunc";
	   funcStyle = true;
	   prefix = true;
	   break;
		   
   case EOpAny:            op = "any";  funcStyle = true; prefix = true; break;
   case EOpAll:            op = "all";  funcStyle = true; prefix = true; break;

      //these are HLSL specific and they map to the lib functions
   case EOpSaturate:
      current->addLibFunction(EOpSaturate);
      op = "xll_saturate";
      funcStyle = true;
      prefix = true;
      break;    

   case EOpTranspose:
      current->addLibFunction(EOpTranspose);
      op = "xll_transpose";
      funcStyle = true;
      prefix = true;
      break;

   case EOpDeterminant:
      current->addLibFunction(EOpDeterminant);
      op = "xll_determinant";
      funcStyle = true;
      prefix = true;
      break;

   case EOpLog10:        
      current->addLibFunction(EOpLog10);
      op = "xll_log10";
      funcStyle = true;
      prefix = true;
      break;       

   case EOpD3DCOLORtoUBYTE4:
      current->addLibFunction(EOpD3DCOLORtoUBYTE4);
      op = "xll_D3DCOLORtoUBYTE4";
      funcStyle = true;
      prefix = true;
      break;

   default:
      assert(0);
   }
   
   // special case for swizzled matrix of matrix index subscripting post/pre increment/decrement
   if ((node->getOp() == EOpPreDecrement || node->getOp() == EOpPreIncrement 
	   || node->getOp() == EOpPostIncrement || node->getOp() == EOpPostDecrement)
	   && node->getOperand()) {
	   TIntermBinary* lval = node->getOperand()->getAsBinaryNode();
	   
	   if (lval)
	   {
		   if( lval->getOp() == EOpMatrixSwizzle) 
		   {
			   static const char* vec_swizzles = "xyzw";
			   TIntermTyped* lexp = lval->getLeft();
			   
			   goit->visitConstant = TGlslOutputTraverser::traverseImmediateConstant;
			   goit->generatingCode = false;
			   
			   lval->getRight()->traverse(goit);
			   
			   goit->visitConstant = TGlslOutputTraverser::traverseConstant;
			   goit->generatingCode = true;
			   
			   std::vector<int> swizzles = goit->indexList;
			   goit->indexList.clear();
			   
			   const char *temp_val = NULL;
			   char *temp_valNameBuffer = NULL;
			   unsigned n_swizzles = swizzles.size();

				//-------------------
				if (goit->parentRequireValue) {
					//create new temporary variable
					temp_valNameBuffer = new char[64];
					snprintf(temp_valNameBuffer, 64, "xlat_swiztemp%d", goit->swizzleAssignTempCounter++);
					temp_val = temp_valNameBuffer;

					//declare it
					current->beginStatement();
					if (n_swizzles == 1)
						out << "float " << temp_val;
					else
						out << "vec" << lval->getNominalSize() << " " << temp_val;
					current->endStatement();
				}
			

				for (unsigned i = 0; i != n_swizzles; ++i) {
				   unsigned col = swizzles[i] / 4;
				   unsigned row = swizzles[i] % 4;
				   
				   current->beginStatement();
				   if (goit->parentRequireValue)
				   {
					   out << temp_val << "." << vec_swizzles[i] << " = " ;
				   }
				   if (prefix)
					   out << op ;
				   lexp->traverse(goit);
				   out << "[" << row << "][" << col << "] ";
				   if (!prefix)
					   out << op;
				   
				   current->endStatement();
				}
				if (goit->parentRequireValue)
				{
					if (goit->tempVariableName.c_str() != temp_val)
						goit->tempVariableName = temp_val;
				}
				else
				{
					goit->tempVariableName = "";
				}


				if (temp_valNameBuffer)
				   delete[] temp_valNameBuffer;

				return false;
			}//if (lval && lval->getOp() == EOpMatrixSwizzle)
		   else if (((lval->getOp() == EOpIndexDirect) || lval->getOp() == EOpIndexIndirect )
			   && !lval->getLeft()->isArray() && lval->getLeft()->isMatrix() )
		   {
				//access row of matrix instead of column

			   TIntermTyped* lexp = lval->getLeft();
			   
			   const char *temp_val;
			   char *temp_valNameBuffer = NULL;
			   char temp_indexval[64];
				
				//--------------------------
				if (goit->parentRequireValue)
				{
					//create new temporary variable
					temp_valNameBuffer = new char[64];
					snprintf(temp_valNameBuffer, 64, "xlat_matRowTemp%d", goit->swizzleAssignTempCounter++);
					temp_val = temp_valNameBuffer;
					//declare it
					current->beginStatement();
					out << "vec" << lval->getNominalSize() << " " << temp_val ;
					current->endStatement();
				}

				TIntermConstant *const_index = lval->getRight()->getAsConstant();
				//calculate index and store in temporary variable
				if (const_index == NULL)
				{
					snprintf(temp_indexval, 64, "xlat_indextemp%d", goit->swizzleAssignTempCounter++);
				
					current->beginStatement();
					if (lval->getRight()->getQualifier() == EvqConst)
						out << "const ";
					out << "int " << temp_indexval << " = ";
					lval->getRight()->traverse(goit);			   
					current->endStatement();
				}
				else
				{
					snprintf(temp_indexval, 64, "%d", const_index->toInt());
				}
			   
			   const char vec_swizzles[] = "xyzw";
			   for (unsigned i = 0; i != lval->getNominalSize(); ++i) {
				   current->beginStatement();
				   if (goit->parentRequireValue)
				   {
					   out << temp_val << "." << vec_swizzles[i] << " = " ;
				   }
				   if (prefix)
					   out << op;
				   lexp->traverse(goit);
				   out << "[" << i << "][" << temp_indexval << "] " ;
				   if (!prefix)
					   out << op;
				   
				   current->endStatement();
			   }

				if (goit->parentRequireValue)
				{
					//use temporary variable as returned value
					if (goit->tempVariableName.c_str() != temp_val)
						goit->tempVariableName = temp_val;
				}
				else
				{
					goit->tempVariableName = "";
				}

			   if (temp_valNameBuffer)
				   delete[] temp_valNameBuffer;
			
			   return false;
		   }//else if (((lval->getOp() == EOpIndexDirect) || lval->getOp() == EOpIndexIndirect )
			//   && !lval->getLeft()->isArray() && lval->getLeft()->isMatrix() )
	   }//if (lval)
	}

	std::string operandStr;
	std::stringstream opOutStream;
	bool needTempVar = false;
	//----traverse operand-------------------
	goit->parentRequireValue = true;//require the operand traversal returns value
	//left
	goit->tempVariableName = "";
	current->setActiveOutput(&opOutStream);
	node->getOperand()->traverse(goit);
	if (goit->tempVariableName.size() > 0)//we have temporary variable
	{
		out << opOutStream.str();//write its calculation first
		operandStr = goit->tempVariableName;
		needTempVar = true;
	}
	else
		operandStr = opOutStream.str();

	//-----------------------------
	current->setActiveOutput(&out);
	needTempVar =  needTempVar && node->getTypePointer()->getBasicType() != EbtVoid && l_parentRequireValue;//really need temp var ?
	
	if (needTempVar)
	{
		current->beginStatement();
		char *tempVar = new char[64];
		snprintf(tempVar, 64, "xlat_unarytemp%d", goit->swizzleAssignTempCounter++);
		goit->tempVariableName = tempVar;
		delete[] tempVar;

		writeTempVarAssign(node->getTypePointer(), goit->tempVariableName, goit, out);
	}

	if (funcStyle)
	  out << op << '(';
	else
	{
	  out << '(';
	  if (prefix)
		 out << op;
	}

	out << operandStr;

	if (! funcStyle && !prefix)
	  out << op;

	out << ')';
	if (needTempVar)
		current->endStatement();
	goit->parentRequireValue = l_parentRequireValue;
	return false;
}


bool TGlslOutputTraverser::traverseSelection( bool preVisit, TIntermSelection *node, TIntermTraverser *it )
{
	TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
	GlslFunction *current = goit->current;
	std::stringstream& out = current->getActiveOutput();
	
	bool l_parentRequireValue = goit->parentRequireValue;
   std::stringstream condOut;

   //-----traverse condition first
   goit->parentRequireValue = true;
   goit->tempVariableName = "";
   current->setActiveOutput(&condOut);
   node->getCondition()->traverse(it);


   //----------------------
   current->setActiveOutput(&out);
   if (goit->tempVariableName.size() > 0)//we have temporary variable
   {
	   out << condOut.str();//write its calculation first
	   current->endStatement();
	   condOut.str(goit->tempVariableName);//then use this variable as condition
   }

	current->beginStatement();

	if (node->getBasicType() == EbtVoid)
	{
		// if/else selection
		out << "if (";
		out << condOut.str();
		out << ')';
		current->beginBlock();
		node->getTrueBlock()->traverse(goit);
		current->endBlock();
		if (node->getFalseBlock())
		{
			current->indent();
			out << "else";
			current->beginBlock();
			node->getFalseBlock()->traverse(goit);
			current->endBlock();
		}
	}
	else 
	{
		std::stringstream trueOut;
		std::stringstream falseOut;
		bool needTempVar = false;
		//traverse true and false block
		goit->parentRequireValue = true;
		//true block
		goit->tempVariableName = "";
		current->setActiveOutput(&trueOut);
		node->getTrueBlock()->traverse(it);
		if (goit->tempVariableName.size() > 0)//we have temporary variable
		{
		   out << trueOut.str();//write its calculation first
		   current->endStatement();
		   current->beginStatement();
		   trueOut.str(goit->tempVariableName);//then use this variable as true selection

		   needTempVar = true;
		}
		//false block
		if (node->getFalseBlock())
		{
			goit->tempVariableName = "";
			current->setActiveOutput(&falseOut);
			node->getFalseBlock()->traverse(it);
			if (goit->tempVariableName.size() > 0)//we have temporary variable
			{
			   out << falseOut.str();//write its calculation first
			   current->endStatement();
			   current->beginStatement();
			   falseOut.str(goit->tempVariableName);//then use this variable as false selection

			   needTempVar = true;
			}
		}

	   //----------------------
		current->setActiveOutput(&out);
		needTempVar = needTempVar && l_parentRequireValue;//really need temp var
		if (needTempVar)
		{
			char * tempVar = new char[64];
			snprintf(tempVar, 64, "xlat_selTemp%d", goit->swizzleAssignTempCounter ++);
			goit->tempVariableName = tempVar;
			delete[] tempVar;

			writeTempVarAssign(node->getTypePointer(), goit->tempVariableName, goit, out);
		}

		if (node->isVector() && node->getCondition()->getAsTyped()->isVector())
		{
			// ?: selection on vectors, e.g. bvec4 ? vec4 : vec4
			// emulate HLSL's component-wise selection here
			current->addLibFunction(EOpVecTernarySel);
			out << "xll_vecTSel (";
			out << condOut.str();
			out << ", ";
			out << trueOut.str();
			out << ", ";
			if (node->getFalseBlock())
			{
				out << falseOut.str();
			}
			else
				assert(0);
			out << ")";
		}
		else
		{
			
		   

			// simple ?: selection
			out << "(( ";
			out << condOut.str();
			out << " ) ? ( ";
			out << trueOut.str();
			out << " ) : ( ";
			if (node->getFalseBlock())
			{
				out << falseOut.str();
			}
			else
				assert(0);
			out << " ))";
		}

		if (needTempVar)
			current->endStatement();
	}
	
	goit->parentRequireValue = l_parentRequireValue;
	return false;
}


bool TGlslOutputTraverser::traverseAggregate( bool preVisit, TIntermAggregate *node, TIntermTraverser *it )
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();
   bool l_parentRequireValue = goit->parentRequireValue;
   int argCount = (int) node->getSequence().size();

   if (node->getOp() == EOpNull)
   {
      goit->infoSink.info << "node is still EOpNull!\n";
      return true;
   }


   switch (node->getOp())
   {
   case EOpSequence:
      if (goit->generatingCode)
      {
		  goit->outputLineDirective (node->getLine());
         TIntermSequence::iterator sit;
         TIntermSequence &sequence = node->getSequence(); 
		 for (sit = sequence.begin(); sit != sequence.end(); ++sit)
		 {
			 if (!goit->isInlining)
				goit->outputLineDirective((*sit)->getLine());
			(*sit)->traverse(it);
		   //out << ";\n";
		   current->endStatement();
		 }
      }
      else
      {
         TIntermSequence::iterator sit;
         TIntermSequence &sequence = node->getSequence(); 
		  for (sit = sequence.begin(); sit != sequence.end(); ++sit)
		  {
		    (*sit)->traverse(it);
		  }
      }

      return false;

   case EOpFunction:
      {
         GlslFunction *func = new GlslFunction( node->getPlainName().c_str(), node->getName().c_str(),
                                                translateType(node->getTypePointer()), goit->m_UsePrecision?node->getPrecision():EbpUndefined,
											   node->getSemantic().c_str(), node->getLine()); 
         if (func->getReturnType() == EgstStruct)
         {
            GlslStruct *s = goit->createStructFromType( node->getTypePointer());
            func->setStruct(s);
         }
         goit->functionList.push_back( func);
         goit->current = func;
		 bool dbgBreak = func->getName().compare("blendTwoWeights") == 0;
         goit->current->beginBlock( false);
         TIntermSequence::iterator sit;
         TIntermSequence &sequence = node->getSequence(); 
		 for (sit = sequence.begin(); sit != sequence.end(); ++sit)
		 {
			 (*sit)->traverse(it);
		 }
         goit->current->endBlock();
         goit->current = goit->global;
         return false;
      }

   case EOpParameters:
      it->visitSymbol = traverseParameterSymbol;
      {
         TIntermSequence::iterator sit;
         TIntermSequence &sequence = node->getSequence(); 
		 for (sit = sequence.begin(); sit != sequence.end(); ++sit)
           (*sit)->traverse(it);
      }
      it->visitSymbol = traverseSymbol;
      return false;

   case EOpConstructFloat: writeFuncCall( "float", node, goit); return false;
   case EOpConstructVec2:  writeFuncCall( "vec2", node, goit); return false;
   case EOpConstructVec3:  writeFuncCall( "vec3", node, goit); return false;
   case EOpConstructVec4:  writeFuncCall( "vec4", node, goit); return false;
   case EOpConstructBool:  writeFuncCall( "bool", node, goit); return false;
   case EOpConstructBVec2: writeFuncCall( "bvec2", node, goit); return false;
   case EOpConstructBVec3: writeFuncCall( "bvec3", node, goit); return false;
   case EOpConstructBVec4: writeFuncCall( "bvec4", node, goit); return false;
   case EOpConstructInt:   writeFuncCall( "int", node, goit); return false;
   case EOpConstructIVec2: writeFuncCall( "ivec2", node, goit); return false;
   case EOpConstructIVec3: writeFuncCall( "ivec3", node, goit); return false;
   case EOpConstructIVec4: writeFuncCall( "ivec4", node, goit); return false;
		   
   case EOpConstructMat2:  writeFuncCall( "mat2", node, goit); return false;
   case EOpConstructMat3:  writeFuncCall( "mat3", node, goit); return false;
   case EOpConstructMat4:  writeFuncCall( "mat4", node, goit); return false;
		   
   case EOpConstructMat2FromMat:
      current->addLibFunction(EOpConstructMat2FromMat);
      writeFuncCall( "xll_constructMat2", node, goit);
      return false;
   case EOpConstructMat2FromNonSquareMat:
	   current->addCalledFunction(node->getName().c_str());
      writeFuncCall( "__constructfloat2x2", node, goit);
      return false;

   case EOpConstructMat3FromMat:
      current->addLibFunction(EOpConstructMat3FromMat);
      writeFuncCall( "xll_constructMat3", node, goit);
      return false;
   case EOpConstructMat3FromNonSquareMat:
	   current->addCalledFunction(node->getName().c_str());
      writeFuncCall( "__constructfloat3x3", node, goit);
      return false;
		   
   case EOpConstructStruct:  writeFuncCall( node->getTypePointer()->getTypeName(), node, goit); return false;
   case EOpConstructArray:  writeFuncCall( buildArrayConstructorString(*node->getTypePointer()), node, goit); return false;

   case EOpComma:
      {
         TIntermSequence::iterator sit;
         TIntermSequence &sequence = node->getSequence(); 
         for (sit = sequence.begin(); sit != sequence.end(); ++sit)
         {
            (*sit)->traverse(it);
            if ( sit+1 != sequence.end())
			{
				if (goit->firstVisitDeclaration)
					current->endStatement();
				else
					out << ", ";
			}
         }
      }
      return false;

   case EOpFunctionCall:
      current->addCalledFunction(node->getName().c_str());
      writeFuncCall( node->getPlainName(), node, goit);
      return false; 

   case EOpLessThan:         writeFuncCall( "lessThan", node, goit); return false;
   case EOpGreaterThan:      writeFuncCall( "greaterThan", node, goit); return false;
   case EOpLessThanEqual:    writeFuncCall( "lessThanEqual", node, goit); return false;
   case EOpGreaterThanEqual: writeFuncCall( "greaterThanEqual", node, goit); return false;
   case EOpVectorEqual:      writeFuncCall( "equal", node, goit); return false;
   case EOpVectorNotEqual:   writeFuncCall( "notEqual", node, goit); return false;

   case EOpMod:
	   current->addLibFunction(EOpMod);
	   writeFuncCall( "xll_mod", node, goit);
	   return false;

   case EOpPow:           writeFuncCall( "pow", node, goit, true); return false;

   case EOpAtan2:         writeFuncCall( "atan", node, goit, true); return false;

   case EOpMin:           writeFuncCall( "min", node, goit, true); return false;
   case EOpMax:           writeFuncCall( "max", node, goit, true); return false;
   case EOpClamp:         writeFuncCall( "clamp", node, goit, true); return false;
   case EOpMix:           writeFuncCall( "mix", node, goit, true); return false;
   case EOpStep:          writeFuncCall( "step", node, goit, true); return false;
   case EOpSmoothStep:    writeFuncCall( "smoothstep", node, goit, true); return false;

   case EOpDistance:      writeFuncCall( "distance", node, goit); return false;
   case EOpDot:           writeFuncCall( "dot", node, goit); return false;
   case EOpCross:         writeFuncCall( "cross", node, goit); return false;
   case EOpFaceForward:   writeFuncCall( "faceforward", node, goit); return false;
   case EOpReflect:       writeFuncCall( "reflect", node, goit); return false;
   case EOpRefract:       writeFuncCall( "refract", node, goit); return false;
   case EOpMul:
      {
         //This should always have two arguments
         assert(node->getSequence().size() == 2);
		 std::string firstOut;//this will store the string representing 1st argument
		 std::stringstream sequenceOut;////this will store the string representing 2nd argument
		 
		 //--------
		 bool needTempVar = false;
		 goit->tempVariableName = "";
		 goit->parentRequireValue = true;
		 current->setActiveOutput(&sequenceOut);
		 //first argument
		 node->getSequence()[0]->traverse(goit);
		 if (goit->tempVariableName.size() > 0)
		 {
			 out << sequenceOut.str();//write temp variable calculation first
			 firstOut = goit->tempVariableName;//then use its name
			 goit->tempVariableName = "";
			 needTempVar = true;
		 }
		 else
			 firstOut = sequenceOut.str();

		 // second argument
		 sequenceOut.str("");
		 node->getSequence()[1]->traverse(goit);
		 if (goit->tempVariableName.size() > 0)
		 {
			 out << sequenceOut.str();//write temp variable calculation first
			 sequenceOut.str( goit->tempVariableName);//then use its name
			 goit->tempVariableName = "";
			 needTempVar = true;
		 }
		
		 //---------------------------
		 current->setActiveOutput(&out);

         current->beginStatement();   

		 if (needTempVar)
		 {
		   char *tempVar = new char[64];
		   snprintf(tempVar, 64, "xlat_aggtemp%d", goit->swizzleAssignTempCounter ++);
		   goit->tempVariableName = tempVar;
		   delete[] tempVar;
		   TType *type = node->getTypePointer();
		   writeTempVarAssign(type, goit->tempVariableName, goit, out);
		 }

         out << '(';
         out << firstOut;
         out << " * ";
		 out << sequenceOut.str();
         out << ')';
		 if (needTempVar)
		 {
			 current->endStatement();
		 }

		 goit->parentRequireValue = l_parentRequireValue; 

         return false;
      }

      //HLSL texture functions
   case EOpTex1D:
      if (argCount == 2)
         writeTex( "texture1D", node, goit);
      else
      {
         current->addLibFunction(EOpTex1DGrad);
         writeTex( "xll_tex1Dgrad", node, goit);
      }
      return false;

   case EOpTex1DProj:     
      writeTex( "texture1DProj", node, goit); 
      return false;

   case EOpTex1DLod:
      current->addLibFunction(EOpTex1DLod);
      writeTex( "xll_tex1Dlod", node, goit); 
      return false;

   case EOpTex1DBias:
      current->addLibFunction(EOpTex1DBias);
      writeTex( "xll_tex1Dbias", node, goit); 
      return false;

   case EOpTex1DGrad:     
      current->addLibFunction(EOpTex1DGrad);
      writeTex( "xll_tex1Dgrad", node, goit); 
      return false;

   case EOpTex2D:
      if (argCount == 2)
         writeTex( "texture2D", node, goit);
      else
      {
         current->addLibFunction(EOpTex2DGrad);
         writeTex( "xll_tex2Dgrad", node, goit);
      }
      return false;

   case EOpTex2DProj:     
      writeTex( "texture2DProj", node, goit);
      return false;

   case EOpTex2DLod:      
      current->addLibFunction(EOpTex2DLod);
      writeTex( "xll_tex2Dlod", node, goit); 
      return false;

   case EOpTex2DBias:  
      current->addLibFunction(EOpTex2DBias);
      writeTex( "xll_tex2Dbias", node, goit); 
      return false;

   case EOpTex2DGrad:  
      current->addLibFunction(EOpTex2DGrad);
      writeTex( "xll_tex2Dgrad", node, goit); 
      return false;

   case EOpTex3D:
      if (argCount == 2)
         writeTex( "texture3D", node, goit);
      else
      {
         current->addLibFunction(EOpTex3DGrad);
         writeTex( "xll_tex3Dgrad", node, goit);            
      }
      return false;

   case EOpTex3DProj:    
      writeTex( "texture3DProj", node, goit); 
      return false;

   case EOpTex3DLod:     
      current->addLibFunction(EOpTex3DLod);
      writeTex( "xll_tex3Dlod", node, goit); 
      return false;

   case EOpTex3DBias:     
      current->addLibFunction(EOpTex3DBias);
      writeTex( "xll_tex3Dbias", node, goit); 
      return false;

   case EOpTex3DGrad:    
      current->addLibFunction(EOpTex3DGrad);
      writeTex( "xll_tex3Dgrad", node, goit); 
      return false;

   case EOpTexCube:
      if (argCount == 2)
         writeTex( "textureCube", node, goit);
      else
      {
         current->addLibFunction(EOpTexCubeGrad);
         writeTex( "xll_texCUBEgrad", node, goit);
      }
      return false;
   case EOpTexCubeProj:   
      writeTex( "textureCubeProj", node, goit); 
      return false;

   case EOpTexCubeLod:    
      current->addLibFunction(EOpTexCubeLod); 
      writeTex( "xll_texCUBElod", node, goit); 
      return false;

   case EOpTexCubeBias:   
      current->addLibFunction(EOpTexCubeBias); 
      writeTex( "xll_texCUBEbias", node, goit); 
      return false;

   case EOpTexCubeGrad:   
      current->addLibFunction(EOpTexCubeGrad);
      writeTex( "xll_texCUBEgrad", node, goit); 
      return false;

   case EOpTexRect:
	   writeTex( "texture2DRect", node, goit);
	   return false;
	   
   case EOpTexRectProj:
	   writeTex( "texture2DRectProj", node, goit);
	   return false;
		   
   case EOpShadow2D:		   
		current->addLibFunction(EOpShadow2D);
		writeTex("xll_shadow2D", node, goit);
		return false;
	   
   case EOpShadow2DProj:
	   current->addLibFunction(EOpShadow2DProj);
	   writeTex("xll_shadow2Dproj", node, goit);
	   return false;
		   
   case EOpModf:
      current->addLibFunction(EOpModf);
      writeFuncCall( "xll_modf", node, goit);
      break;

   case EOpLdexp:
      current->addLibFunction(EOpLdexp);
      writeFuncCall( "xll_ldexp", node, goit);
      break;

   case EOpSinCos:        
      current->addLibFunction(EOpSinCos);
      writeFuncCall ( "xll_sincos", node, goit);
      break;

   case EOpLit:
      current->addLibFunction(EOpLit);
      writeFuncCall( "xll_lit", node, goit);
      break;

   default: goit->infoSink.info << "Bad aggregation op\n";
   }


   return false;
}


bool TGlslOutputTraverser::traverseLoop( bool preVisit, TIntermLoop *node, TIntermTraverser *it )
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();
   bool l_parentRequireValue = goit->parentRequireValue;

   //traverse the condition
   std::stringstream condout;
   current->setActiveOutput(&condout);
   goit->tempVariableName = "";
   goit->parentRequireValue = true;
   if (node->getCondition())
		node->getCondition()->traverse(goit);
   bool conditionTempVar = goit->tempVariableName.size() > 0;
   std::string tempCondChildVar = goit->tempVariableName;
   
   
   //-------------------------
   current->setActiveOutput(&out);

   current->beginStatement();

   TLoopType loopType = node->getType();
   if (loopType == ELoopFor)
   {
      // Process for loop, initial statement was promoted outside the loop
      out << "for ( ; ";
      if (node->getCondition())
	  {
		  if (conditionTempVar)//hack, set loop condition to true and inside the body, break if temp var is false
			out << "true";
		  else
			out << condout.str();
	  }
      out << "; ";
      if (node->getExpression())
         node->getExpression()->traverse(goit);
      out << ") ";
      current->beginBlock();
	  if (conditionTempVar)//hack, set loop condition to true and inside the body, break if temp var is false
	  {
		  out << condout.str();//write temp var calculation
		  out << "if (!bool(" << goit->tempVariableName << ")) break;\n";
	  }
      if (node->getBody())
         node->getBody()->traverse(goit);
      current->endBlock();
   }
   else if (loopType == ELoopWhile)
      {
         // Process while loop
         out << "while ( ";
		  if (node->getCondition())
		  {
			  if (conditionTempVar)//hack, set loop condition to true and inside the body, break if temp var is false
				out << "true";
			  else
				out << condout.str();
		  }
         out << " ) ";
         current->beginBlock();
		 if (conditionTempVar)//hack, set loop condition to true and inside the body, break if temp var is false
		  {
			  out << condout.str();//write temp var calculation
			  out << "if (!bool(" << goit->tempVariableName << ")) break;\n";
		  }
         if (node->getBody())
            node->getBody()->traverse(goit);
         current->endBlock();
      }
      else
      {
		assert(loopType == ELoopDoWhile);
         // Process do loop
		std::string tempCondVar = "";
		 if (conditionTempVar)
		 {
			 char *tempCondVarBuffer = new char[64];
			 snprintf(tempCondVarBuffer, 64, "xlat_condTemp%d", goit->swizzleAssignTempCounter++);
			 tempCondVar = tempCondVarBuffer;
			 delete[] tempCondVarBuffer;

			 //declare it
			 out << "bool " + tempCondVar;
			 current->endStatement();
			 current->beginStatement();
		 }
         out << "do ";
         current->beginBlock();
         if (node->getBody())
            node->getBody()->traverse(goit);
		 if (conditionTempVar)
		 {
			 //write temp var calculation
			 out << condout.str();
			 //assign it to the temp conditional var
			 out << tempCondVar << " = bool(" + tempCondChildVar + ");\n";
		 }
         current->endBlock();
         current->indent();
         out << "while ( ";
		 if (conditionTempVar)//hack, set loop condition to true and inside the body, break if temp var is false
			out << tempCondVar;
		  else
			out << condout.str();
         out << " );\n";
      }

   goit->parentRequireValue = l_parentRequireValue;
   return false;
}


bool TGlslOutputTraverser::traverseBranch( bool preVisit, TIntermBranch *node,  TIntermTraverser *it )
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();

   bool l_parentRequireValue = goit->parentRequireValue;
   std::stringstream exprOut;

   //-----traverse expression first
   goit->parentRequireValue = true;
   goit->tempVariableName = "";
   current->setActiveOutput(&exprOut);

   if (node->getExpression())
   {
      node->getExpression()->traverse(it);
   }

   //----------------------
   current->setActiveOutput(&out);
   if (goit->tempVariableName.size() > 0)//we have temporary variable
   {
	   out << exprOut.str();//write its calculation first
	   exprOut.str(goit->tempVariableName);//then use this variable as expression
   }

   if (goit->isInlining && node->getFlowOp() == EOpReturn && node->getExpression())//inline expanding
   {
	   current->beginStatement();
	   out << *goit->inlineRetVar << " = " << exprOut.str();
   }
   else
   {
	   current->beginStatement();

	   switch (node->getFlowOp())
	   {
	   case EOpKill:      out << "discard";           break;
	   case EOpBreak:     out << "break";          break;
	   case EOpContinue:  out << "continue";       break;
	   case EOpReturn:    out << "return ";         break;
	   default:           assert(0); break;
	   }

	   if (node->getExpression())
	   {
		   out << exprOut.str();
	   }

   }
	goit->parentRequireValue = l_parentRequireValue;
	goit->tempVariableName = "";

   return false;
}


GlslStruct *TGlslOutputTraverser::createStructFromType (TType *type)
{
   GlslStruct *s = 0;
   std::string structName = type->getTypeName().c_str();

   //check for anonymous structures
   if (structName.size() == 0)
   {
      std::stringstream temp;
      TTypeList &tList = *type->getStruct();

      //build a mangled name that is hopefully mangled enough to prevent collisions
      temp << "anonStruct";

      for (TTypeList::iterator it = tList.begin(); it != tList.end(); it++)
      {
         TString typeString;
         it->type->buildMangledName(typeString);
         temp << "_" << typeString.c_str();
      }

      structName = temp.str();
   }

   //try to find the struct name
   if ( structMap.find(structName) == structMap.end() )
   {
      //This is a new structure, build a type for it
      TTypeList &tList = *type->getStruct();

      s = new GlslStruct(structName, type->getLine());

      for (TTypeList::iterator it = tList.begin(); it != tList.end(); it++)
      {
         GlslStruct::StructMember m;
         m.name = it->type->getFieldName().c_str();

         if (it->type->hasSemantic())
            m.semantic = it->type->getSemantic().c_str();

        if (it->type->getBasicType() == EbtStruct)
        {
            m.structType = createStructFromType(it->type);
        }
        else
            m.structType = NULL;

         m.type = translateType( it->type);
         m.arraySize = it->type->isArray() ? it->type->getArraySize() : 0;
		 m.precision = m_UsePrecision ? it->type->getPrecision() : EbpUndefined;
         s->addMember(m);
      }

      //add it to the list
      structMap[structName] = s;
      structList.push_back(s);
   }
   else
   {
      s = structMap[structName];
   }

   return s;
}
