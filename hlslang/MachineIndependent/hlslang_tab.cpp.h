typedef union {
    struct {
        TSourceLoc line;
        union {
            TString *string;
            float f;
            int i;
            bool b;
        };
        TSymbol* symbol;
    } lex;
    struct {
        TSourceLoc line;
        TOperator op;
        union {
            TIntermNode* intermNode;
            TIntermNodePair nodePair;
            TIntermTyped* intermTypedNode;
            TIntermAggregate* intermAggregate;
			TIntermDeclaration* intermDeclaration;
        };
        union {
            TPublicType type;
            TQualifier qualifier;
            TFunction* function;
            TParameter param;
            TTypeLine typeLine;
            TTypeList* typeList;
	    TAnnotation* ann;
	    TTypeInfo* typeInfo;
        };
    } interm;
} YYSTYPE;
#define	ATTRIBUTE	258
#define	CONST_QUAL	259
#define	STATIC_QUAL	260
#define	BOOL_TYPE	261
#define	FLOAT_TYPE	262
#define	INT_TYPE	263
#define	STRING_TYPE	264
#define	FIXED_TYPE	265
#define	HALF_TYPE	266
#define	BREAK	267
#define	CONTINUE	268
#define	DO	269
#define	ELSE	270
#define	FOR	271
#define	IF	272
#define	DISCARD	273
#define	RETURN	274
#define	BVEC2	275
#define	BVEC3	276
#define	BVEC4	277
#define	IVEC2	278
#define	IVEC3	279
#define	IVEC4	280
#define	VEC2	281
#define	VEC3	282
#define	VEC4	283
#define	HVEC2	284
#define	HVEC3	285
#define	HVEC4	286
#define	FVEC2	287
#define	FVEC3	288
#define	FVEC4	289
#define	MATRIX2	290
#define	MATRIX3	291
#define	MATRIX4	292
#define	HMATRIX2	293
#define	HMATRIX3	294
#define	HMATRIX4	295
#define	FMATRIX2	296
#define	FMATRIX3	297
#define	FMATRIX4	298
#define	IN_QUAL	299
#define	OUT_QUAL	300
#define	INOUT_QUAL	301
#define	UNIFORM	302
#define	VARYING	303
#define	STRUCT	304
#define	VOID_TYPE	305
#define	WHILE	306
#define	SAMPLER1D	307
#define	SAMPLER2D	308
#define	SAMPLER3D	309
#define	SAMPLERCUBE	310
#define	SAMPLER1DSHADOW	311
#define	SAMPLER2DSHADOW	312
#define	SAMPLERRECTSHADOW	313
#define	SAMPLERRECT	314
#define	SAMPLERGENERIC	315
#define	VECTOR	316
#define	MATRIX	317
#define	REGISTER	318
#define	TEXTURE	319
#define	SAMPLERSTATE	320
#define	IDENTIFIER	321
#define	TYPE_NAME	322
#define	FLOATCONSTANT	323
#define	INTCONSTANT	324
#define	BOOLCONSTANT	325
#define	STRINGCONSTANT	326
#define	FIELD_SELECTION	327
#define	LEFT_OP	328
#define	RIGHT_OP	329
#define	INC_OP	330
#define	DEC_OP	331
#define	LE_OP	332
#define	GE_OP	333
#define	EQ_OP	334
#define	NE_OP	335
#define	AND_OP	336
#define	OR_OP	337
#define	XOR_OP	338
#define	MUL_ASSIGN	339
#define	DIV_ASSIGN	340
#define	ADD_ASSIGN	341
#define	MOD_ASSIGN	342
#define	LEFT_ASSIGN	343
#define	RIGHT_ASSIGN	344
#define	AND_ASSIGN	345
#define	XOR_ASSIGN	346
#define	OR_ASSIGN	347
#define	SUB_ASSIGN	348
#define	LEFT_PAREN	349
#define	RIGHT_PAREN	350
#define	LEFT_BRACKET	351
#define	RIGHT_BRACKET	352
#define	LEFT_BRACE	353
#define	RIGHT_BRACE	354
#define	DOT	355
#define	COMMA	356
#define	COLON	357
#define	EQUAL	358
#define	SEMICOLON	359
#define	BANG	360
#define	DASH	361
#define	TILDE	362
#define	PLUS	363
#define	STAR	364
#define	SLASH	365
#define	PERCENT	366
#define	LEFT_ANGLE	367
#define	RIGHT_ANGLE	368
#define	VERTICAL_BAR	369
#define	CARET	370
#define	AMPERSAND	371
#define	QUESTION	372

