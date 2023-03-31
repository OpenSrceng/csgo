//============ Copyright (c) Valve Corporation, All rights reserved. ============

#ifndef ISCENESYSTEM
#define ISCENESYSTEM

#ifdef _WIN32
#pragma once
#endif

#include "appframework/iappsystem.h"
#include "mathlib/camera.h"
#include "rendersystem/irenderdevice.h"
#include "rendersystem/irendercontext.h"
#include "rendersystem/indexdata.h"
#include "rendersystem/vertexdata.h"
#include "particles/particles.h"
#include "rendersystem/schema/renderbuffer.g.h"
#include "rendersystem/schema/renderable.g.h"

//---------------------------------------------------------------------------------------------------------------------------------------------------
// Forward declarations
//---------------------------------------------------------------------------------------------------------------------------------------------------

class CSceneObject;											// something that can be rendered
class CSceneParticleObject;
class CSceneMonitorObject;

class CSceneObjectList;										// a set of objects which pass some
															// test (for example, being in the
															// frustum. generated by traversal

class CSceneView;											// describes one scene render, such as
															// a splitscreen view, a shadow map
															// frustum, etc.


enum MaterialDrawMode_t
{
	MATDRAWMODE_REGULAR = 0,								// full forward rendering
	MATDRAWMODE_LIGHTPREPASS = 1,							// output to a gbuffer
	MATDRAWMODE_LIGHTPOSTPASS = 2,							// 2nd pass deferred
	MATDRAWMODE_NUMMODES
};

// placeholder
struct Mat2DrawModeDescriptor_t
{
	HRenderTextureStrong m_hTextureToBind;
	RenderShaderHandle_t m_hVertexShader;
	RenderShaderHandle_t m_hPixelShader;
};

class IMat2
{
public:
	Mat2DrawModeDescriptor_t m_drawDescriptors[MATDRAWMODE_NUMMODES];
	RenderInputLayout_t m_hLayout;
	uint m_nPassFlags;										// SCENEOBJECTFLAG_ flags about which material this pass renders in.

	void Bind( IRenderContext *pCtx, MaterialDrawMode_t nMode, RenderInputLayout_t hLayout = RENDER_INPUT_LAYOUT_INVALID );


};
struct ModelVertexXFormStream_t
{
	Vector4D m_matTransform0;
	Vector4D m_matTransform1;
	Vector4D m_matTransform2;
};

struct CMeshDrawPrimitive_t
{
	uint32 m_nSortKey;
	matrix3x4_t *m_pTransform;
	MaterialDrawDescriptor_t const *m_pDrawOp;
	CSceneObject *m_pObject;
};

class ISceneObjectDesc
{
public:
	virtual CSceneObject *Create( void ) = 0;

	virtual void DrawArray( class IRenderContext *pCtx, CMeshDrawPrimitive_t *pRenderList, int nNumRenderablesToDraw,
							class CSceneView const *pView, class CSceneLayer *pLayer ) {}
	
	// generate the simple static vb primitives for this object. may be called with a non 0
	// nStartPrimitive multiple times if there is a buffer overflow (this should be a rare
	// occurrence
	virtual int GeneratePrimitives( CSceneObject *pObject, int nStartPrimitive, int nMaxOutputPrimitives, CMeshDrawPrimitive_t *pOutBuf,
									CSceneView const *pView, CSceneLayer *pLayer ) { return 0; }
	
};



class CSceneLayer;

typedef void (*LAYERDRAWFN)( class ISceneView *pView, IRenderContext *pCtx, CSceneLayer *pLayer );


class ISceneView											// this class is going to become concrete
{
public:

	virtual CSceneLayer *AddRenderLayer( RenderViewport_t const &viewport, MaterialDrawMode_t eShaderMode ) =0;
	virtual CSceneLayer *AddProceduralLayer( RenderViewport_t const &viewport, LAYERDRAWFN pFnToRender ) =0;

	virtual ~ISceneView( void ) {}
};


abstract_class ISceneSystem : public IAppSystem
{
public:

	// creating renderables
	virtual CSceneObject *CreateSceneObject( ISceneObjectDesc *pDesc, uint nFlags ) =0;

	// finding and adding renderable types
	virtual ISceneObjectDesc *GetSceneObjectDesc( char const *pName ) = 0;
	
	virtual void AddSceneObjectType( char const *pName, ISceneObjectDesc *pDescriptor ) =0;


	virtual CSceneObject *CreateSceneObject( uint nFlags ) =0;


	// rendering. First call beginRenderingViews, then call AddRenerView on all of the views you
	// want to render, and then call FinishRenderingViews which will kick off all of the threads.
	// since rendering is asynchronous, you need to call WaitForRenderingToComplete() before
	// starting another render or changing the state of sceneobjects, etc.

	virtual void BeginRenderingViews( IRenderDevice *pDevice ) = 0;

	virtual ISceneView *AddView( CFrustum const &frustum ) = 0;
	
	virtual void BeginRenderingDynamicView( ISceneView *pView ) = 0; // only call this on views created during rendering

	virtual void FinishRenderingViews( void ) = 0;

	virtual void WaitForRenderingToComplete( void ) = 0;


	virtual void InitSystem( void ) =0;


	// general obejct stuff
	virtual void SetObjectBounds( CSceneObject *pObj, Vector const &vecMins, Vector const &vecMaxes ) =0;

	// particles
	virtual CSceneParticleObject *CreateParticleObject( const char *pParticleSystemName, float flDelay = 0.0f, int nRandomSeed = 0 ) =0;
	virtual CSceneMonitorObject *CreateMonitorObject( void ) =0;



	virtual CParticleSystemMgr *ParticleMgr( void ) =0;

	virtual bool ReadParticleConfigFile( const char *pFileName, bool bPrecache, bool bDecommitTempMemory = true ) =0;

	// create a particle system by name. returns null if one of that name does not exist
	virtual CParticleCollection *CreateParticleCollection( const char *pParticleSystemName, float flDelay = 0.0f, int nRandomSeed = 0 ) =0;
	virtual CParticleCollection *CreateParticleCollection( ParticleSystemHandle_t particleSystemName, float flDelay = 0.0f, int nRandomSeed = 0 ) =0;

	virtual void Shutdown( void ) =0;

	virtual ~ISceneSystem( void ) {}
};





#endif // ISCENESYSTEM
