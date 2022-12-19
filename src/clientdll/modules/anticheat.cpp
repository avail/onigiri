#include <stdinc.hpp>

#include <utils/static_initializer.hpp>
#include <utils/Hooking.h>

#include <gta/gameSkeleton.hpp>

static std::uint32_t sus_2802_hash = 0xA0F39FB6;

static std::unordered_map<uint32_t, std::string> g_initFunctionNames;

namespace rage
{
	/*const char* InitFunctionTypeToString(InitFunctionType type)
	{
		switch (type)
		{
		case INIT_CORE:
			return "INIT_CORE";

		case INIT_BEFORE_MAP_LOADED:
			return "INIT_BEFORE_MAP_LOADED";

		case INIT_AFTER_MAP_LOADED:
			return "INIT_AFTER_MAP_LOADED";

		case INIT_SESSION:
			return "INIT_SESSION";
		}

		return "INIT_UNKNOWN";
	}*/

	const char* InitFunctionData::GetName() const
	{
		auto it = g_initFunctionNames.find(funcHash);

		if (it != g_initFunctionNames.end())
		{
			return it->second.c_str();
		}

		return std::format("0x{:08x}", funcHash).c_str();
	}

	bool InitFunctionData::TryInvoke(InitFunctionType type)
	{
		if (funcHash == HashString("fwClothMeshing")
			|| funcHash == HashString("rageSecEngine")
			|| funcHash == HashString("CCreditsText")
			|| funcHash == HashString("rageSecGamePluginManager")
			|| funcHash == HashString("TamperActions")
			|| funcHash == sus_2802_hash)
		{
			onigiri::services::logger::info("not invoking ac init func");
			return false;
		}

		initFunction(type);
		return true;
	}

	void gameSkeleton::RunInitFunctions(InitFunctionType type)
	{
		//onigiri::services::logger::info(__FUNCTION__ ": Running %s init functions\n", InitFunctionTypeToString(type));

		for (auto list = m_initFunctionList; list; list = list->next)
		{
			if (list->type == type)
			{
				for (auto entry = list->entries; entry; entry = entry->next)
				{
					//onigiri::services::logger::info(__FUNCTION__ ": Running functions of order %i (%i total)\n", entry->order, entry->functions.GetCount());
					int i = 0;

					for (int index : entry->functions)
					{
						auto func = m_initFunctions[index];

						//onigiri::services::logger::info(__FUNCTION__ ": Invoking %s %s init (%i out of %i)\n", func.GetName(), InitFunctionTypeToString(type), i + 1, entry->functions.GetCount());

						func.TryInvoke(type);

						++i;
					}
				}
			}
		}

		//onigiri::services::logger::info(__FUNCTION__ ": Done running %s init functions!\n", InitFunctionTypeToString(type));
	}

	static void RunEntries(gameSkeleton_updateBase* update)
	{
		for (auto entry = update; entry; entry = entry->m_nextPtr)
		{
			if (/*entry->m_hash == HashString("fwClothMeshing")
				||*/ entry->m_hash == HashString("rageSecEngine")
				//|| entry->m_hash == HashString("CCreditsText")
				//|| entry->m_hash == HashString("rageSecGamePluginManager")
				//|| entry->m_hash == HashString("TamperActions")
				|| entry->m_hash == sus_2802_hash)
			{
				return;
			}

			//onigiri::services::logger::debug(std::format("running update func 0x{:08x}", entry->m_hash).c_str());
			entry->Run();
		}
	}

	void gameSkeleton::RunUpdate(int type)
	{
		for (auto list = m_updateFunctionList; list; list = list->next)
		{
			if (list->type == type)
			{
				RunEntries(list->entry);
			}
		}
	}

	void gameSkeleton_updateBase::RunGroup()
	{
		RunEntries(this->m_childPtr);
	}
}

static const char* const g_initFunctionKnown[] = {
	"AmbientLights",
	"AnimBlackboard",
	"Audio",
	"BackgroundScripts",
	"CActionManager",
	"CAgitatedManager",
	"CAmbientAnimationManager",
	"CAmbientAudioManager",
	"CAmbientModelSetManager",
	"CAnimBlackboard",
	"CAppDataMgr",
	"CAssistedMovementRouteStore",
	"CBoatChaseDirector",
	"CBuses",
	"CBusySpinner",
	"CCheat",
	"CCheckCRCs",
	"CClipDictionaryStoreInterface",
	"CClock",
	"CCombatDirector",
	"CCombatInfoMgr",
	"CCompEntity",
	"CConditionalAnimManager",
	"CContentExport",
	"CContentSearch",
	"CControl",
	"CControlMgr",
	"CControllerLabelMgr",
	"CCover",
	"CCoverFinder",
	"CCredits",
	"CCrimeInformationManager",
	"CCullZones",
	"CDLCScript",
	"CDecoratorInterface",
	"CDispatchData",
	"CEventDataManager",
	"CExpensiveProcessDistributer",
	"CExplosionManager",
	"CExtraContent",
	"CExtraContentWrapper",
	"CExtraContentWrapper::Shutdown",
	"CExtraContentWrapper::ShutdownStart",
	"CExtraMetadataMgr",
	"CExtraMetadataMgr::ClassInit",
	"CExtraMetadataMgr::ClassShutdown",
	"CExtraMetadataMgr::ShutdownDLCMetaFiles",
	"CFlyingVehicleAvoidanceManager",
	"CFocusEntityMgr",
	"CFrontendStatsMgr",
	"CGameLogic",
	"CGameSituation",
	"CGameStreamMgr",
	"CGameWorld",
	"CGameWorldHeightMap",
	"CGameWorldWaterHeight",
	"CGarages",
	"CGenericGameStorage",
	"CGestureManager",
	"CGps",
	"CGtaAnimManager",
	"CHandlingDataMgr",
	"CInstanceListAssetLoader::Init",
	"CInstanceListAssetLoader::Shutdown",
	"CIplCullBox",
	"CJunctions",
	"CLODLightManager",
	"CLODLights",
	"CLadderMetadataManager",
	"CLoadingScreens",
	"CMapAreas",
	"CMapZoneManager",
	"CMessages",
	"CMiniMap",
	"CModelInfo",
	"CModelInfo::Init",
	"CMovieMeshManager",
	"CMultiplayerGamerTagHud",
	"CNetRespawnMgr",
	"CNetwork",
	"CNetworkTelemetry",
	"CNewHud",
	"CObjectPopulationNY",
	"COcclusion",
	"CParaboloidShadow",
	"CPathFind",
	"CPathServer::InitBeforeMapLoaded",
	"CPathServer::InitSession",
	"CPathServer::ShutdownSession",
	"CPathZoneManager",
	"CPatrolRoutes",
	"CPauseMenu",
	"CPed",
	"CPedAILodManager",
	"CPedGeometryAnalyser",
	"CPedModelInfo",
	"CPedPopulation",
	"CPedPopulation::ResetPerFrameScriptedMu",
	"CPedPopulation::ResetPerFrameScriptedMultipiers",
	"CPedPropsMgr",
	"CPedVariationPack",
	"CPedVariationStream",
	"CPerformance",
	"CPhoneMgr",
	"CPhotoManager",
	"CPhysics",
	"CPickupDataManager",
	"CPickupManager",
	"CPlantMgr",
	"CPlayStats",
	"CPlayerSwitch",
	"CPopCycle",
	"CPopZones",
	"CPopulationStreaming",
	"CPopulationStreamingWrapper",
	"CPortal",
	"CPortalTracker",
	"CPostScan",
	"CPrecincts",
	"CPrioritizedClipSetRequestManager",
	"CPrioritizedClipSetStreamer",
	"CProcObjectMan",
	"CProceduralInfo",
	"CProfileSettings",
	"CRandomEventManager",
	"CRecentlyPilotedAircraft",
	"CRenderPhaseCascadeShadowsInterface",
	"CRenderTargetMgr",
	"CRenderThreadInterface",
	"CRenderer",
	"CReportMenu",
	"CRestart",
	"CRiots",
	"CRoadBlock",
	"CScaleformMgr",
	"CScenarioActionManager",
	"CScenarioManager",
	"CScenarioManager::ResetExclusiveScenari",
	"CScenarioManager::ResetExclusiveScenarioGroup",
	"CScenarioPointManager",
	"CScenarioPointManagerInitSession",
	"CScene",
	"CSceneStreamerMgr::PreScanUpdate",
	"CScriptAreas",
	"CScriptCars",
	"CScriptDebug",
	"CScriptEntities",
	"CScriptHud",
	"CScriptPedAIBlips",
	"CScriptPeds",
	"CScriptedGunTaskMetadataMgr",
	"CShaderHairSort",
	"CShaderLib",
	"CSituationalClipSetStreamer",
	"CSky",
	"CSlownessZonesManager",
	"CSprite2d",
	"CStaticBoundsStore",
	"CStatsMgr",
	"CStreaming",
	"CStreamingRequestList",
	"CStuntJumpManager",
	"CTVPlaylistManager",
	"CTacticalAnalysis",
	"CTask",
	"CTaskClassInfoManager",
	"CTaskRecover",
	"CTexLod",
	"CText",
	"CThePopMultiplierAreas",
	"CTheScripts",
	"CTimeCycle",
	"CTrafficLights",
	"CTrain",
	"CTuningManager",
	"CUserDisplay",
	"CVehicleAILodManager",
	"CVehicleChaseDirector",
	"CVehicleCombatAvoidanceArea",
	"CVehicleDeformation",
	"CVehicleMetadataMgr",
	"CVehicleModelInfo",
	"CVehiclePopulation",
	"CVehiclePopulation::ResetPerFrameScript",
	"CVehiclePopulation::ResetPerFrameScriptedMultipiers",
	"CVehicleRecordingMgr",
	"CVehicleVariationInstance",
	"CVisualEffects",
	"CWarpManager",
	"CWaypointRecording",
	"CWeaponManager",
	"CWitnessInformationManager",
	"CWorldPoints",
	"CZonedAssetManager",
	"Common",
	"CreateFinalScreenRenderPhaseList",
	"Credits",
	"CutSceneManager",
	"CutSceneManagerWrapper",
	"FacialClipSetGroupManager",
	"FireManager",
	"FirstPersonProp",
	"FirstPersonPropCam",
	"Game",
	"GenericGameStoragePhotoGallery",
	"INSTANCESTORE",
	"ImposedTxdCleanup",
	"InitSystem",
	"Kick",
	"LightEntityMgr",
	"Lights",
	"MeshBlendManager",
	"Misc",
	"NewHud",
	"Occlusion",
	"PauseMenu",
	"Ped",
	"PedHeadShotManager",
	"PedModelInfo",
	"PedPopulation",
	"PlantsMgr::UpdateBegin",
	"PlantsMgr::UpdateEnd",
	"Population",
	"PostFX",
	"PostFx",
	"Pre-vis",
	"Prioritized",
	"Proc",
	"ProcessAfterCameraUpdate",
	"ProcessAfterMovement",
	"ProcessPedsEarlyAfterCameraUpdate",
	"Render",
	"ResetSceneLights",
	"Run",
	"Script",
	"ScriptHud",
	"ShaderLib::Update",
	"Situational",
	"SocialClubMenu",
	"Streaming",
	"UI3DDrawManager",
	"UIWorldIconManager",
	"Update",
	"VehPopulation",
	"VideoPlayback",
	"VideoPlaybackThumbnailManager",
	"VideoPlaybackThumbnails",
	"Viewport",
	"ViewportSystemInit",
	"ViewportSystemInitLevel",
	"ViewportSystemShutdown",
	"ViewportSystemShutdownLevel",
	"Visibility",
	"Visual",
	"WarningScreen",
	"Water",
	"WaterHeightSim",
	"World",
	"audNorthAudioEngine",
	"audNorthAudioEngineDLC",
	"cStoreScreenMgr",
	"camManager",
	"decorators",
	"fwAnimDirector",
	"fwClipSetManager",
	"fwClothStore",
	"fwDrawawableStoreWrapper",
	"fwDwdStore",
	"fwDwdStoreWrapper",
	"fwExpressionSetManager",
	"fwFacialClipSetGroupManager",
	"fwFragmentStoreWrapper",
	"fwMapTypesStore",
	"fwMetaDataStore",
	"fwTimer",
	"fwTxdStore",
	"perfClearingHouse",
	"strStreamingEngine::SubmitDeferredAsyncPlacementRequests",

	"CAnimSceneManager",
	"CTextInputBox",
	"CMultiplayerChat",
	"CCreditsText",
	"CReplayMgr",
	"CReplayCoordinator",
	"CMousePointer",
	"CVideoEditorUI",
	"CVideoEditorInterface",
	"VideoRecording",
	"WatermarkRenderer",
};

static onigiri::utils::static_initializer _([]()
{
	for (auto str : g_initFunctionKnown)
	{
		g_initFunctionNames.insert({ HashString(str), str });
		g_initFunctionNames.insert({ HashRageString(str), str });
	}

	onigiri::services::logger::info("disabling rage::RageSecurity~");

	void* loc = hook::pattern("BA 04 00 00 00 E8 ? ? ? ? E8 ? ? ? ? E8").count(1).get(0).get<void>(5);

	hook::jump(hook::get_call(loc), hook::get_member(&rage::gameSkeleton::RunInitFunctions));

	hook::jump(hook::get_call(hook::get_pattern("48 8D 0D ? ? ? ? BA 02 00 00 00 84 DB 75 05", -17)), hook::get_member(&rage::gameSkeleton::RunUpdate));

	hook::jump(hook::get_pattern("40 53 48 83 EC 20 48 8B 59 20 EB 0D 48 8B 03 48"), hook::get_member(&rage::gameSkeleton_updateBase::RunGroup));

	/*static auto security = hook::get_address<rage::atSingleton<rage::RageSecurity>*>(hook::get_pattern("48 8B ? ? ? ? ? 33 F6 E9 ? ? ? ? 55 48 8D ? ? ? ? ? 48 87 2C 24 C3 48 8B 45 50 0F B6 00", 3));

	const auto handle = CreateThread(nullptr, 0, [](auto module) -> DWORD
	{
		std::this_thread::sleep_for(10s);

		while (g_running)
		{
			if (auto instance = security->getInstance(); security->isValid())
			{
				instance->m_interval = std::numeric_limits<uint32_t>::max();
			}

			std::this_thread::yield();
		}

		return 0;
	}, nullptr, 0, nullptr);*/
}, INT32_MIN);
