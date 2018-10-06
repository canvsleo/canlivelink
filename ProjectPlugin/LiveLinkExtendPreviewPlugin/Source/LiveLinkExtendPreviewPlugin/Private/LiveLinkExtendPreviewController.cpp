
/*! @file		LiveLinkExtendPreviewController.cpp
	@brief		LiveLinkExtendを使用した際のメッシュプレビュー中の挙動 
	@details	LiveLinkPreviewController.cpp をもとにした実装となっており、\n
				カメラの画角とメッシュ更新が拡張されております。
*/


#include "LiveLinkExtendPreviewController.h"
#include "ILiveLinkClient.h"
#include "LiveLinkTypes.h"

#include "LiveLinkInstance.h"
#include "LiveLinkRemapAsset.h"

#include "CameraController.h"
#include "IPersonaPreviewScene.h"
#include "Animation/DebugSkelMeshComponent.h"

#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"

#include "MeshUtilities.h"

#include "Animation/MorphTarget.h"

#include "LiveLinkExtendVertexMeshData.h"


#include "Serialization/MemoryReader.h"


const FName EditorCamera( "EditorActiveCamera" );
const FName EditorCameraMetaData_FOV( "fov" );
const FName EditorMesh_MetaData_MeshSyncData( "mesh_sync_data" );
const FName EditorMesh_MetaData_SendTime( "send_time" );


/*------- ↓ 作業用データ ↓ ------- {{{ */

	/*! @struct		MorphDeltaPosition
		@brief		１頂点ごとのモーフターゲットの移動量
	*/
	struct MorphDeltaPosition
	{
		MorphDeltaPosition()
			: VertexIndex( 0 )
		{}

		MorphDeltaPosition(
			const FVector&	position,
			uint32			vertexIndex
		)
			: Position		( position )
			, VertexIndex	( vertexIndex )
		{}

		FVector		Position;
		uint32		VertexIndex;
	};

	/*! @struct		MorphData
		@brief		１モーフターゲットの作成内容
	*/
	struct MorphData
	{

		MorphData()
			: MorphTarget( nullptr )
		{}

		UMorphTarget*					MorphTarget;
		TArray< MorphDeltaPosition >	DeltaPositionList;
};

/*------- ↑ 作業用データ ↑ ------- }}} */






FLiveLinkExtendInstanceProxy::FLiveLinkExtendInstanceProxy()
	: PreviewMeshComponent( nullptr )
{
}

FLiveLinkExtendInstanceProxy::FLiveLinkExtendInstanceProxy( UAnimInstance* InAnimInstance )
	: FLiveLinkInstanceProxy( InAnimInstance )
	, PreviewMeshComponent( nullptr )
{
}

FLiveLinkExtendInstanceProxy::~FLiveLinkExtendInstanceProxy()
{}

void	FLiveLinkExtendInstanceProxy::Initialize( UAnimInstance* InAnimInstance )
{
	FLiveLinkInstanceProxy::Initialize( InAnimInstance );
}

bool	FLiveLinkExtendInstanceProxy::Evaluate( FPoseContext& Output )
{
	auto ret = FLiveLinkInstanceProxy::Evaluate( Output );

	ILiveLinkClient* client = this->clientRef_.GetClient();
	IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>( "MeshUtilities" );
	if( 
		client &&
		this->PreviewMeshComponent &&
		this->PreviewMeshComponent->SkeletalMesh
	)
	{
		auto skeltalMesh = this->PreviewMeshComponent->SkeletalMesh;

		

		auto model = skeltalMesh->GetImportedModel();
		const FLiveLinkSubjectFrame* frame = client->GetSubjectData( this->PoseNode.SubjectName );
		if( 
			model &&
			model->LODModels.Num() > 0 &&
			frame
		)
		{
			auto meshSyncBase64	= frame->MetaData.StringMetaData.Find( EditorMesh_MetaData_MeshSyncData );
			auto sendTime		= frame->MetaData.StringMetaData.Find( EditorMesh_MetaData_SendTime );
			FDateTime semdTimeStamp;

			TArray<uint8> meshSyncBuffer;

			if( 
				// DCCツールとの連携はBase64化された FLiveLinkExtendSyncData バイナリを使用する
				meshSyncBase64 &&
				FBase64::Decode( *meshSyncBase64, meshSyncBuffer ) &&
				FDateTime::ParseIso8601( **sendTime, semdTimeStamp )
			)
			{
				FMemoryReader readBuffer( meshSyncBuffer );

				FLiveLinkExtendSyncData syncData;
				syncData << readBuffer;
				if( 
					// 送信データのタイムスタンプにて更新の必要を判断
					this->lastReceiveTime_ < semdTimeStamp 
				)
				{

					this->lastReceiveTime_ = semdTimeStamp;

					if( skeltalMesh->MorphTargets.Num() > 0 )
					{
						skeltalMesh->UnregisterAllMorphTarget();
					}

					if( syncData.BoneBindPoseList.Num() > 0 )
					{
						FReferenceSkeletonModifier skeletonModifier( skeltalMesh->RefSkeleton, skeltalMesh->Skeleton );


						const auto& boneInfoList = skeltalMesh->RefSkeleton.GetRefBoneInfo();
						auto numBones = skeltalMesh->RefSkeleton.GetNum();
						for( int iBone = 0; iBone < numBones; ++iBone )
						{
							const auto& boneInfo	= boneInfoList[ iBone ];

							for( const auto& boneBindPose : syncData.BoneBindPoseList )
							{
								if( boneInfo.Name.ToString() == boneBindPose.MeshName )
								{
									skeletonModifier.UpdateRefPoseTransform( iBone, boneBindPose.BindPose );
								}
							}
						}
					}

					skeltalMesh->bHasVertexColors = 0;

					TArray<FVector>			lodPoints;
					TArray<FMeshWedge>		lodWedges;
					TArray<FMeshFace>		lodFaces;
					TArray<FVertInfluence>	lodInfluences;
					TArray<int32>			lodPointToRawMap;

					TMap< FString, MorphData >	morphDataMap;

					auto& lodModel = model->LODModels[0];

					int numSections = lodModel.Sections.Num();
					int numVertexies	= lodModel.NumVertices;

					for( int32 iSection = 0; iSection < numSections; ++iSection )
					{
						FSkelMeshSection& section = lodModel.Sections[ iSection ];

						const auto& material = skeltalMesh->Materials[ section.MaterialIndex ];

						bool findUpdateMesh = false;

						for( const auto& mesh : syncData.MeshList )
						{
							this->lastMeshUpdateTimeMap_.Emplace( mesh.MeshName, mesh.TimeStamp );

							int numGroups = mesh.MaterialGroupList.Num();
										

							int currentVertexBase	= lodPoints.Num();
							int numVertexPoints		= mesh.PointList.Num();

							for( const auto& point : mesh.PointList )
							{
								lodPoints.Add(
									FVector(
										point.X,
										point.Y,
										point.Z
									)
								);
							}

							TArray<uint32>		polygonIndexList;
							TArray<FVector2D>	faceIndexUVs;
							TArray<uint32>		faceInSmoothingGroupIndexList;

							if( mesh.NumUseColors > 0 )
							{
								skeltalMesh->bHasVertexColors = 1;
							}

							for( const auto& materialGroup : mesh.MaterialGroupList )
							{

								bool sameMaterial = false;
								{
									// セクションのマテリアル名と同一名のメッシュグループデータからメッシュ構築を行う
									if( materialGroup.MaterialName == material.MaterialSlotName.ToString() )
									{
										sameMaterial = true;
									}
									else if( materialGroup.MaterialName == material.MaterialInterface->GetName() )
									{
										sameMaterial = true;
									}
								}

								if( sameMaterial )
								{
									{
										// ボーンから各頂点の影響を取り出し
										const auto& boneList = skeltalMesh->RefSkeleton.GetRawRefBoneInfo();
										auto numSkinBondes = mesh.SkinNameList.Num();

										const int InvalidBone = -1;
										TArray<int> boneMap;
										boneMap.Init( InvalidBone, numSkinBondes );
										for( int iLocalBone = 0; iLocalBone < numSkinBondes; ++iLocalBone )
										{
											for( int iBone = 0; iBone < boneList.Num(); ++iBone )
											{
												if( *mesh.SkinNameList[ iLocalBone ] == boneList[ iBone ].Name )
												{
													boneMap[ iLocalBone ] = iBone;
													break;
												}
											}
										}

										auto numSkinPoints = mesh.PointSkinList.Num();
										if( numSkinPoints == numVertexPoints )
										{
											for( int iSkinPoint = 0; iSkinPoint < numSkinPoints; ++iSkinPoint )
											{
												auto skinObject = mesh.PointSkinList[ iSkinPoint ];

												TArray<FVertInfluence> workInfluence;

												auto totalWeight = 0.0f;
												for( const auto& skinObj : skinObject.SkinList )
												{
													if( boneMap[ skinObj.SkinIndex ] != InvalidBone )
													{
														auto weight = skinObj.SkinWeight;
														if( weight < 0.1 )
														{
															// 低すぎる比率のボーン影響を与えると 頂点フォーマットから精度落ちしてしまうため回避
															continue;
														}

														FVertInfluence influence;
														influence.BoneIndex = boneMap[ skinObj.SkinIndex ];
														influence.VertIndex	= currentVertexBase + iSkinPoint;

														influence.Weight	= weight;
														totalWeight += influence.Weight;
														workInfluence.Add( influence );
													}
												}

												if( totalWeight  > 0.0f )
												{
													workInfluence.Sort(
														[]( const FVertInfluence& a, const FVertInfluence& b ){
														return a.Weight > b.Weight; 
													});
													for( auto& influence : workInfluence )
													{
														influence.Weight /= totalWeight;
														lodInfluences.Add( influence );
													}	
												}
											}
															
										}
										else
										{
											// 頂点数が一致しない場合にはルートか参照メッシュそのものをスキンの参照元とする
											int boneIndex = 0;
											for( int iBone = 0; iBone < boneList.Num(); ++iBone )
											{
												if( mesh.MeshName == boneList[ iBone ].Name.ToString() )
												{
													boneIndex = iBone;
													break;
												}
											}

											for( int iVertex = 0; iVertex < numVertexPoints; ++iVertex )
											{
												FVertInfluence influence;
												influence.BoneIndex = boneIndex;
												influence.VertIndex	= currentVertexBase + iVertex;
												influence.Weight	= 1.0f;
												lodInfluences.Add( influence );
											}
										}
									}


									for( const auto& polygon : materialGroup.PolygonList )
									{
										// ポリゴン形成

										auto numVertices		= polygon.VertexIndexList.Num();
										for( int iFace = 0; iFace < numVertices; iFace+=3 )
										{
											FMeshFace face;
											FMemory::Memset( &face, 0, sizeof( face ) );

											for( int iTriangle = 0; iTriangle < 3; ++iTriangle )
											{
												auto vertexIndex = polygon.VertexIndexList[ iFace + iTriangle ];

												polygonIndexList.Add( vertexIndex );

												FMeshWedge wedge;
												FMemory::Memset( &wedge, 0, sizeof( wedge ) );
												wedge.iVertex	= vertexIndex + currentVertexBase;
												wedge.Color		= FColor::White;

												if( mesh.NumUseColors > 0 )
												{
													const auto& colorObj = polygon.ColorList0[ iFace + iTriangle ];
													{
														wedge.Color.R = ( uint8 )( colorObj.X * 255.0 );
														wedge.Color.G = ( uint8 )( colorObj.Y * 255.0 );
														wedge.Color.B = ( uint8 )( colorObj.Z * 255.0 );
														wedge.Color.A = ( uint8 )( colorObj.W * 255.0 );
													}
												}

												if( 
													mesh.NumUseUVs >= 1 &&
													polygon.UVList0.Num() > 0 
												)
												{
													const auto& uvObject = polygon.UVList0[ iFace + iTriangle ];
													{
														wedge.UVs[ 0 ].X = uvObject.X;
														wedge.UVs[ 0 ].Y = 1.0f - uvObject.Y;
													}
												}
												if( 
													mesh.NumUseUVs >= 2 &&
													polygon.UVList1.Num() > 0 
												)
												{
													const auto& uvObject = polygon.UVList1[ iFace + iTriangle ];
													{
														wedge.UVs[ 1 ].X = uvObject.X;
														wedge.UVs[ 1 ].Y = 1.0f - uvObject.Y;
													}
												}
												if(
													mesh.NumUseUVs >= 3 &&
													polygon.UVList2.Num() > 0 
												)
												{
													const auto& uvObject = polygon.UVList2[ iFace + iTriangle ];
													{
														wedge.UVs[ 2 ].X = uvObject.X;
														wedge.UVs[ 2 ].Y = 1.0f - uvObject.Y;
													}
												}
												if( 
													mesh.NumUseUVs >= 4 &&
													polygon.UVList3.Num() > 0 
												)
												{
													const auto& uvObject = polygon.UVList3[ iFace + iTriangle ];
													{
														wedge.UVs[ 3 ].X = uvObject.X;
														wedge.UVs[ 3 ].Y = 1.0f - uvObject.Y;
													}
												}

												face.iWedge[ iTriangle ] = lodWedges.Add( wedge );

												face.TangentX[ iTriangle ] = FVector::ZeroVector;
												face.TangentY[ iTriangle ] = FVector::ZeroVector;
												face.TangentZ[ iTriangle ] = FVector::ZeroVector;

												if( polygon.NormalList.Num() > 0 )
												{
													const auto& normalObj = polygon.NormalList[ iFace + iTriangle ];
													{
														face.TangentZ[ iTriangle ] = 
															FVector(
																normalObj.X,
																normalObj.Y,
																normalObj.Z
															);
													}
												}
												else if( mesh.PointNormalList.Num() > 0 && vertexIndex < mesh.PointNormalList.Num() )
												{
													const auto& normalObj = mesh.PointNormalList[ vertexIndex ];
													{
														face.TangentZ[ iTriangle ] = 
															FVector(
																normalObj.X,
																normalObj.Y,
																normalObj.Z
															);
													}
												}

												faceIndexUVs.Add( wedge.UVs[ 0 ] );
											}
											faceInSmoothingGroupIndexList.Add( polygon.SmoothingGroup );


											face.MeshMaterialIndex = section.MaterialIndex;
																
											face.SmoothingGroups = polygon.SmoothingGroup;
											lodFaces.Add( face );
										}
									}


									auto numMorph = mesh.MorphList.Num();
									if( numMorph > 0 )
									{
										// モーフターゲットの頂点対応のみ取り出しておく
										for( int iMorph = 0; iMorph < numMorph; ++iMorph )
										{
											const auto& morphObject = mesh.MorphList[ iMorph ];
											if( morphObject.VertexList.Num() > 0 )
											{
												MorphData* morphData = morphDataMap.Find( morphObject.MorphName );
												if( !morphData )
												{
													MorphData newMorphData;
													newMorphData.MorphTarget = NewObject<UMorphTarget>( skeltalMesh, FName( *morphObject.MorphName ) );
													morphDataMap.Add( morphObject.MorphName, newMorphData );
													morphData = morphDataMap.Find( morphObject.MorphName );
												}


												for( int iVertex = 0; iVertex < morphObject.VertexList.Num(); ++iVertex )
												{
													uint32 vertexIndex = iVertex + currentVertexBase;
													const auto& vertexData = morphObject.VertexList[ iVertex ];

													morphData->DeltaPositionList.Add(
														MorphDeltaPosition(
															FVector(
																vertexData.DeltaPoint.X,
																vertexData.DeltaPoint.Y,
																vertexData.DeltaPoint.Z
															),
															vertexIndex
														)
													);
												}
											}
										}
									}
									findUpdateMesh = true;
								}
							}
						}


						if( !findUpdateMesh )
						{
							// セクションに対応するメッシュデータが存在しない場合は、
							// セクション内のデータから同一のデータを再生成させるように
							int currentVertexBase = lodPoints.Num();
							int numVertex = section.SoftVertices.Num();

							for( int iVertex = 0; iVertex < numVertex; ++iVertex )
							{
								const auto& vertex = section.SoftVertices[ iVertex ];
								lodPoints.Add( FVector( vertex.Position.X, vertex.Position.Y, vertex.Position.Z ) );

								int vertexIndex = iVertex + currentVertexBase;

								for( int iInFluence = 0; iInFluence < section.MaxBoneInfluences; ++iInFluence )
								{
									FVertInfluence influence;
									influence.BoneIndex = vertex.InfluenceBones[ iInFluence ];
									influence.VertIndex	= vertexIndex;
									influence.Weight	= ( float )vertex.InfluenceWeights[ iInFluence ] / 255.0f;

									lodInfluences.Add( influence );
								}
							}

							for( uint32 iTriangle = 0; iTriangle < section.NumTriangles; ++iTriangle )
							{

								FMeshFace face;
								for( int iVertex = 0; iVertex < 3; ++iVertex )
								{
									int vertexIndex = lodModel.IndexBuffer[ iTriangle * 3 + iVertex + section.BaseIndex ] - section.BaseVertexIndex;

									const auto& vertex = section.SoftVertices[ vertexIndex ];

									FMeshWedge wedge;
									wedge.iVertex	= vertexIndex + currentVertexBase;
									wedge.Color		= vertex.Color;

									FMemory::Memcpy( wedge.UVs, vertex.UVs, sizeof( vertex.UVs ) );
									face.iWedge[ iVertex ] = lodWedges.Add( wedge );


									face.TangentX[ iVertex ] = vertex.TangentX;
									face.TangentY[ iVertex ] = vertex.TangentY;
									face.TangentZ[ iVertex ] = vertex.TangentZ;

								}

								face.MeshMaterialIndex	= section.MaterialIndex;
								face.SmoothingGroups	= 255;
								lodFaces.Add( face );
							}
						}
					}



					lodPointToRawMap.AddZeroed( lodPoints.Num() );
					for( int iPoint = 0; iPoint < lodPointToRawMap.Num(); ++iPoint )
					{
						lodPointToRawMap[ iPoint ] = iPoint;
					}

					IMeshUtilities::MeshBuildOptions buildOptions;
					{
						buildOptions.bComputeNormals			= false;
						buildOptions.bComputeTangents			= true;
						buildOptions.bUseMikkTSpace				= true;
						buildOptions.bRemoveDegenerateTriangles = false;
					}

					TArray<FText> warningMessages;
					TArray<FName> warningNames;

					bool buildSuccess = 
						MeshUtilities.BuildSkeletalMesh( 
							model->LODModels[0],
							skeltalMesh->RefSkeleton, 
							lodInfluences, lodWedges, lodFaces, lodPoints, lodPointToRawMap, 
							buildOptions,
							&warningMessages, &warningNames
						);
					if( buildSuccess )
					{
						{
							auto numMorphTargets = morphDataMap.Num();
							if( numMorphTargets > 0 )
							{
								// skeltalMeshの頂点バッファ用にdeltaを構築して、モーフターゲットを適応
								for( const auto& morphDataPair : morphDataMap )
								{
									TArray< FMorphTargetDelta > deltaDataList;

									auto numImportVertex = model->LODModels[ 0 ].MeshToImportVertexMap.Num();

									for( const auto& morphDeltaPoint : morphDataPair.Value.DeltaPositionList )
									{
										for( int iImportedVertexIndex = 0; iImportedVertexIndex < numImportVertex; ++iImportedVertexIndex )
										{
											if( model->LODModels[ 0 ].MeshToImportVertexMap[ iImportedVertexIndex ] == morphDeltaPoint.VertexIndex )
											{
												FMorphTargetDelta delta;

												delta.PositionDelta = morphDeltaPoint.Position;
												delta.TangentZDelta	= FVector::ZeroVector;
												delta.SourceIdx		= iImportedVertexIndex;

												deltaDataList.Add( delta );
											}
										}
									}
									morphDataPair.Value.MorphTarget->PopulateDeltas(
										deltaDataList,
										0, model->LODModels[ 0 ].Sections,
										true
									);
									if( morphDataPair.Value.MorphTarget->HasValidData() )
									{
										skeltalMesh->RegisterMorphTarget( morphDataPair.Value.MorphTarget );
									}
								}
								skeltalMesh->ReleaseResources();
							}
						}
						
						{
							// バウンドの更新
							FBox boundingBox( lodPoints.GetData(), lodPoints.Num() );
							FBox workBox = boundingBox;
							FVector midMesh = 0.5f*( workBox.Min + workBox.Max );
							boundingBox.Min = workBox.Min + 1.0f*( workBox.Min - midMesh );
							boundingBox.Max = workBox.Max + 1.0f*( workBox.Max - midMesh );

							skeltalMesh->SetImportedBounds( FBoxSphereBounds( boundingBox ) );
						}
						
						FPropertyChangedEvent emptyPropertyUpdateStruct( NULL );
						//emptyPropertyUpdateStruct.ChangeType = EPropertyChangeType::Interactive;
						skeltalMesh->PostEditChangeProperty( emptyPropertyUpdateStruct );
						skeltalMesh->PostLoad();
					}
				}
			}
		}
	}
	return ret;
}

void	FLiveLinkExtendInstanceProxy::UpdateAnimationNode( float DeltaSeconds )
{
	FLiveLinkInstanceProxy::UpdateAnimationNode( DeltaSeconds );
}




/*! @class		FLiveLinkExtendCameraController
	@brief		カメラへ同期への操作
*/
class FLiveLinkExtendCameraController 
	: public FEditorCameraController
{
	FLiveLinkClientReference ClientRef;

public:


	/*! @brief	カメラ同期への反映
		@note	FLiveLinkCameraControllerでの挙動とは異なり、\n
				画角設定を行う
	*/
	virtual void UpdateSimulation(
		const FCameraControllerUserImpulseData& UserImpulseData,
		const float DeltaTime,
		const bool bAllowRecoilIfNoImpulse,
		const float MovementSpeedScale,
		FVector& InOutCameraPosition,
		FVector& InOutCameraEuler,
		float& InOutCameraFOV)
	{
		if (ILiveLinkClient* Client = ClientRef.GetClient())
		{
			if (const FLiveLinkSubjectFrame* Frame = Client->GetSubjectData(EditorCamera))
			{
				if( Frame->Transforms.IsValidIndex( 0 ) )
				{
					FTransform Camera = Frame->Transforms[0];
					InOutCameraPosition	= Camera.GetLocation();
					InOutCameraEuler	= Camera.GetRotation().Euler();

					auto findFOV = Frame->MetaData.StringMetaData.Find( EditorCameraMetaData_FOV );
					if( findFOV )
					{
						InOutCameraFOV = FCString::Atof( **findFOV );
					}
					return;
				}
			}
		}

		InOutCameraPosition	= FVector(0.f);
		InOutCameraEuler	= FVector(0.f);
	}
	
};









ULiveLinkExtendInstance::ULiveLinkExtendInstance( const FObjectInitializer& Initializer )
	: Super( Initializer )
{
}

FAnimInstanceProxy* ULiveLinkExtendInstance::CreateAnimInstanceProxy()
{
	return new FLiveLinkExtendInstanceProxy( this );
}










void ULiveLinkExtendPreviewController::InitializeView(UPersonaPreviewSceneDescription* SceneDescription, IPersonaPreviewScene* PreviewScene) const
{
	auto previewMeshComponent = PreviewScene->GetPreviewMeshComponent();
	if( previewMeshComponent )
	{
		previewMeshComponent->SetAnimInstanceClass( ULiveLinkExtendInstance::StaticClass() );

		if( ULiveLinkExtendInstance* LiveLinkInstance = Cast<ULiveLinkExtendInstance>( previewMeshComponent->GetAnimInstance() ) )
		{
			LiveLinkInstance->SetSubject( SubjectName );
			LiveLinkInstance->SetRetargetAsset( RetargetAsset );
			LiveLinkInstance->SetPreviewMeshComponent( previewMeshComponent );
		}
	}
	if (bEnableCameraSync)
	{
		PreviewScene->SetCameraOverride(MakeShared<FLiveLinkExtendCameraController>());
	}
}

void ULiveLinkExtendPreviewController::UninitializeView(UPersonaPreviewSceneDescription* SceneDescription, IPersonaPreviewScene* PreviewScene) const
{
	PreviewScene->SetCameraOverride(nullptr);
	auto previewMeshComponent = PreviewScene->GetPreviewMeshComponent();
	if( previewMeshComponent )
	{
		previewMeshComponent->SetAnimInstanceClass( nullptr );
	}
}


