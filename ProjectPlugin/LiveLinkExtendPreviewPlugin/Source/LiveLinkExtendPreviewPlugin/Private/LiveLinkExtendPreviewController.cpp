


#include "LiveLinkExtendPreviewController.h"
#include "ILiveLinkClient.h"
#include "LiveLinkTypes.h"

#include "LiveLinkInstance.h"
#include "LiveLinkRemapAsset.h"

#include "CameraController.h"
#include "IPersonaPreviewScene.h"
#include "Animation/DebugSkelMeshComponent.h"

#include "PhysicsEngine/BodySetup.h"

#include "PhysXCookHelper.h"

#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"

#include "MeshUtilities.h"

#include "Animation/MorphTarget.h"


const FName EditorCamera ( TEXT( "EditorActiveCamera" ) );
const FName EditorCameraMetaData_FOV( TEXT( "fov" ) );
const FName EditorMesh_MetaData_MeshSyncData( TEXT( "mesh_sync_data" ) );
const FName EditorMesh_MetaData_SendTime( TEXT( "send_time" ) );


void	FLiveLinkExtendInstanceProxy::Initialize( UAnimInstance* InAnimInstance )
{
	FLiveLinkInstanceProxy::Initialize( InAnimInstance );
}

bool	FLiveLinkExtendInstanceProxy::Evaluate( FPoseContext& Output )
{
	auto ret = FLiveLinkInstanceProxy::Evaluate( Output );

	ILiveLinkClient* client = this->ClientRef.GetClient();
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
			auto meshSyncJson = frame->MetaData.StringMetaData.Find( EditorMesh_MetaData_MeshSyncData );
			auto sendTime = frame->MetaData.StringMetaData.Find( EditorMesh_MetaData_SendTime );
			FDateTime semdTimeStamp;

			if( 
				meshSyncJson && 
				FDateTime::ParseIso8601( **sendTime, semdTimeStamp )
			)
			{
				TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create( *meshSyncJson ); 
				TSharedPtr<FJsonObject> meshSyncData = MakeShareable( new FJsonObject );
				if( 
					this->lastReceiveTime_ < semdTimeStamp &&
					FJsonSerializer::Deserialize( JsonReader, meshSyncData ) 
				)
				{
					this->lastReceiveTime_ = semdTimeStamp;
					//frame->MetaData.StringMetaData.Remove( EditorMesh_MetaData_MeshSyncData );


					TArray<FVector>			lodPoints;
					TArray<FMeshWedge>		lodWedges;
					TArray<FMeshFace>		lodFaces;
					TArray<FVertInfluence>	lodInfluences;
					TArray<int32>			lodPointToRawMap;

					if( meshSyncData->HasField( "MeshList" ) )
					{
						auto& lodModel = model->LODModels[0];

						int numSections = lodModel.Sections.Num();

						int numVertexies	= lodModel.NumVertices;

						FMatrix matrix;
						matrix.M[ 0 ][ 0 ] = 1.0f;
						matrix.M[ 0 ][ 1 ] = 0.0f;
						matrix.M[ 0 ][ 2 ] = 0.0f;
						matrix.M[ 0 ][ 3 ] = 0.0f;

						matrix.M[ 1 ][ 0 ] = 0.0f;
						matrix.M[ 1 ][ 1 ] = 0.0f;
						matrix.M[ 1 ][ 2 ] = 1.0f;
						matrix.M[ 1 ][ 3 ] = 0.0f;

						matrix.M[ 2 ][ 0 ] = 0.0f;
						matrix.M[ 2 ][ 1 ] =-1.0f;
						matrix.M[ 2 ][ 2 ] = 0.0f;
						matrix.M[ 2 ][ 3 ] = 0.0f;

						matrix.M[ 3 ][ 0 ] = 0.0f;
						matrix.M[ 3 ][ 1 ] = 0.0f;
						matrix.M[ 3 ][ 2 ] = 0.0f;
						matrix.M[ 3 ][ 3 ] = 1.0f;


						auto meshList = meshSyncData->GetArrayField( "MeshList" );

						for( int32 iSection = 0; iSection < numSections; ++iSection )
						{
							FSkelMeshSection& section = lodModel.Sections[ iSection ];

							const auto& material = skeltalMesh->Materials[ section.MaterialIndex ];

							bool findUpdateMesh = false;

							for( const auto& mesh : meshList )
							{
								const TSharedPtr<FJsonObject>* meshData = nullptr;
								if( 
									mesh->TryGetObject( meshData ) &&
									( *meshData )->HasField( "PointList" ) &&
									( *meshData )->HasField( "PointSkinList" ) &&
									( *meshData )->HasField( "PointNormalList" ) &&
									( *meshData )->HasField( "SkinNameList" ) &&
									( *meshData )->HasField( "MaterialGroupList" )
								)
								{
									auto meshName		= ( *meshData )->GetStringField( "MeshName" );
									auto timeStampStr	= ( *meshData )->GetStringField( "TimeStamp" );

									const auto& pointArray			= ( *meshData )->GetArrayField( "PointList" );
									const auto& pointSkinArray		= ( *meshData )->GetArrayField( "PointSkinList" );
									const auto& pointNormalList		= ( *meshData )->GetArrayField( "PointNormalList" );

									const auto& skinNameList		= ( *meshData )->GetArrayField( "SkinNameList" );

									const auto& materialGroupList	= ( *meshData )->GetArrayField( "MaterialGroupList" );

									FDateTime timeStamp;
									if( FDateTime::ParseIso8601( *timeStampStr, timeStamp ) )
									{
										auto lastUpdateTime = this->lastMeshUpdateTimeMap_.Find( meshName );
										if( lastUpdateTime )
										{
											if( *lastUpdateTime >= timeStamp )
											{
												//continue;
											}
										}

										this->lastMeshUpdateTimeMap_.Emplace( meshName, timeStamp );

										int numPoints = pointArray.Num();
										int numGroups = materialGroupList.Num();

										UMorphTarget* morphTarget = FindObject<UMorphTarget>( skeltalMesh, *meshName );
										if( morphTarget )
										{
											//! TODO : Not Implemented.
										}
										
										for( const auto& materialGroup : materialGroupList )
										{
											auto materialGroupObj = materialGroup->AsObject();
											if( 
												materialGroupObj.IsValid() &&
												materialGroupObj->HasField( "Triangles" ) &&
												materialGroupObj->HasField( "MaterialName" )
											)
											{
												auto triangles		= materialGroupObj->GetIntegerField( "Triangles" );
												auto materialName	= materialGroupObj->GetStringField( "MaterialName" );

												if(
													materialName == material.MaterialSlotName.ToString() &&
													materialGroupObj->HasField( "PolygonList" )
												)
												{
													const auto& polygonList = materialGroupObj->GetArrayField( "PolygonList" );

													int currentVertexBase	= lodPoints.Num();
													int numVertexPoints		= pointArray.Num();

													for( const auto& point : pointArray )
													{
														auto pointObj = point->AsObject();
														if( pointObj.IsValid() )
														{
															auto vec = 
																FVector4( 
																	pointObj->GetNumberField( "X" ),
																	pointObj->GetNumberField( "Y" ),
																	pointObj->GetNumberField( "Z" )
																);

															auto resultVec	= matrix.TransformFVector4( vec );
															resultVec.Y		= -resultVec.Y;
															lodPoints.Add( resultVec );
														}
													}


													{
														const auto& boneList = skeltalMesh->RefSkeleton.GetRawRefBoneInfo();
														auto numSkinBondes = skinNameList.Num();

														const int InvalidBone = -1;
														TArray<int> boneMap;
														boneMap.Init( InvalidBone, numSkinBondes );
														for( int iLocalBone = 0; iLocalBone < numSkinBondes; ++iLocalBone )
														{
															for( int iBone = 0; iBone < boneList.Num(); ++iBone )
															{
																if( *skinNameList[ iLocalBone ]->AsString() == boneList[ iBone ].Name )
																{
																	boneMap[ iLocalBone ] = iBone;
																	break;
																}
															}
														}

														auto numSkinPoints = pointSkinArray.Num();
														if( numSkinPoints == numVertexPoints )
														{
															for( int iSkinPoint = 0; iSkinPoint < numSkinPoints; ++iSkinPoint )
															{
																auto skinObject = pointSkinArray[ iSkinPoint ]->AsObject();
																if( skinObject.IsValid() )
																{
																	auto skinList = skinObject->GetArrayField( "SkinList" );
																	auto numSkin = skinList.Num();

																	TArray<FVertInfluence> workInfluence;

																	auto totalWeight = 0.0f;
																	for( int iSkin = 0; iSkin < numSkin; ++iSkin )
																	{
																		auto skinObj = skinList[ iSkin ]->AsObject();
																		if( skinObj.IsValid() )
																		{
																			auto localBoneIndex = skinObj->GetIntegerField( "SkinIndex" );
																			if( boneMap[ localBoneIndex ] != InvalidBone )
																			{
																				auto weight = skinObj->GetNumberField( "SkinWeight" );
																				if( weight < 0.1 )
																				{
																					continue;
																				}

																				FVertInfluence influence;
																				influence.BoneIndex = boneMap[ localBoneIndex ];
																				influence.VertIndex	= currentVertexBase + iSkinPoint;

																				influence.Weight	= weight;
																				totalWeight += influence.Weight;
																				workInfluence.Add( influence );
																			}
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
															
														}
														else
														{
															int boneIndex = 0;
															for( int iBone = 0; iBone < boneList.Num(); ++iBone )
															{
																if( meshName == boneList[ iBone ].Name.ToString() )
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


													for( auto polygon : polygonList )
													{
														auto polygonObj = polygon->AsObject();
														if( 
															polygonObj.IsValid() &&
															polygonObj->HasField( "VertexIndexList" ) &&
															polygonObj->HasField( "SmoothingGroup" ) &&
															polygonObj->HasField( "ColorList0" ) &&
															polygonObj->HasField( "NormalList" ) &&
															polygonObj->HasField( "UVList0" ) &&
															polygonObj->HasField( "UVList1" ) &&
															polygonObj->HasField( "UVList2" ) &&
															polygonObj->HasField( "UVList3" )
														)
														{
															auto vertexIndexList	= polygonObj->GetArrayField( "VertexIndexList" );

															auto smoothingGroup	= polygonObj->GetIntegerField( "SmoothingGroup" );

															auto colorList0	= polygonObj->GetArrayField( "ColorList0" );
															auto normalList	= polygonObj->GetArrayField( "NormalList" );
															auto uvList0	= polygonObj->GetArrayField( "UVList0" );
															auto uvList1	= polygonObj->GetArrayField( "UVList1" );
															auto uvList2	= polygonObj->GetArrayField( "UVList2" );
															auto uvList3	= polygonObj->GetArrayField( "UVList3" );

															auto numVertices		= vertexIndexList.Num();
															
															for( int iFace = 0; iFace < numVertices; iFace+=3 )
															{
																FMeshFace face;

																FMemory::Memset( &face, 0, sizeof( face ) );

																for( int iTriangle = 0; iTriangle < 3; ++iTriangle )
																{
																	int vertexIndex = FCString::Atoi( *( vertexIndexList[ iFace + iTriangle ])->AsString() );

																	FMeshWedge wedge;
																	wedge.iVertex	= vertexIndex + currentVertexBase;
																	wedge.Color		= FColor::White;

																	auto colorObj = colorList0[ iFace + iTriangle ]->AsObject();
																	if( colorObj.IsValid() )
																	{
																		wedge.Color.R = colorObj->GetNumberField( "X" );
																		wedge.Color.G = colorObj->GetNumberField( "Y" );
																		wedge.Color.B = colorObj->GetNumberField( "Z" );
																		wedge.Color.A = colorObj->GetNumberField( "W" );
																	}

																	if( uvList0.Num() > 0 )
																	{
																		auto uvObject = uvList0[ iFace + iTriangle ]->AsObject();
																		if( uvObject.IsValid() )
																		{
																			wedge.UVs[ 0 ].X = uvObject->GetNumberField( "X" );
																			wedge.UVs[ 0 ].Y = 1.0f - uvObject->GetNumberField( "Y" );
																		}
																	}
																	if( uvList1.Num() > 0 )
																	{
																		auto uvObject = uvList1[ iFace + iTriangle ]->AsObject();
																		if( uvObject.IsValid() )
																		{
																			wedge.UVs[ 1 ].X = uvObject->GetNumberField( "X" );
																			wedge.UVs[ 1 ].Y = 1.0f - uvObject->GetNumberField( "Y" );
																		}
																	}
																	if( uvList2.Num() > 0 )
																	{
																		auto uvObject = uvList2[ iFace + iTriangle ]->AsObject();
																		if( uvObject.IsValid() )
																		{
																			wedge.UVs[ 2 ].X = uvObject->GetNumberField( "X" );
																			wedge.UVs[ 2 ].Y = 1.0f - uvObject->GetNumberField( "Y" );
																		}
																	}
																	if( uvList3.Num() > 0 )
																	{
																		auto uvObject = uvList3[ iFace + iTriangle ]->AsObject();
																		if( uvObject.IsValid() )
																		{
																			wedge.UVs[ 3 ].X = uvObject->GetNumberField( "X" );
																			wedge.UVs[ 3 ].Y = 1.0f - uvObject->GetNumberField( "Y" );
																		}
																	}

																	face.iWedge[ iTriangle ] = lodWedges.Add( wedge );

																	face.TangentX[ iTriangle ] = FVector::ZeroVector;
																	face.TangentY[ iTriangle ] = FVector::ZeroVector;
																	face.TangentZ[ iTriangle ] = FVector::ZeroVector;

																	if( normalList.Num() > 0 )
																	{
																		auto normalObj = normalList[ iFace + iTriangle ]->AsObject();
																		if( normalObj.IsValid() )
																		{
																			auto vec =
																				FVector4(
																					normalObj->GetNumberField( "X" ),
																					normalObj->GetNumberField( "Y" ),
																					normalObj->GetNumberField( "Z" )
																				);

																			auto resultVec	= matrix.TransformFVector4( vec );
																			resultVec.Y		= -resultVec.Y;
																			face.TangentZ[ iTriangle ] = resultVec;
																		}
																	}
																	else if( pointNormalList.Num() > 0 && vertexIndex < pointNormalList.Num() )
																	{
																		auto normalObj = pointNormalList[ vertexIndex ]->AsObject();
																		if( normalObj.IsValid() )
																		{
																			auto vec =
																				FVector4(
																					normalObj->GetNumberField( "X" ),
																					normalObj->GetNumberField( "Y" ),
																					normalObj->GetNumberField( "Z" )
																				);

																			auto resultVec	= matrix.TransformFVector4( vec );
																			resultVec.Y		= -resultVec.Y;
																			face.TangentZ[ iTriangle ] = resultVec;
																		}
																	}
																}

																face.MeshMaterialIndex = section.MaterialIndex;
																
																face.SmoothingGroups = smoothingGroup;
																lodFaces.Add( face );
															}
														}
													}

													findUpdateMesh = true;
												}
											}
										}
									}
								}
							}


							if( !findUpdateMesh )
							{
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

					}


					lodPointToRawMap.AddZeroed( lodPoints.Num() );
					for( int iPoint = 0; iPoint < lodPointToRawMap.Num(); ++iPoint )
					{
						lodPointToRawMap[ iPoint ] = iPoint;
					}


					IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>( "MeshUtilities" );

					IMeshUtilities::MeshBuildOptions buildOptions;
					{
						buildOptions.bComputeNormals			= false;
						buildOptions.bComputeTangents			= true;
						buildOptions.bUseMikkTSpace				= true;
						buildOptions.bRemoveDegenerateTriangles = false;
					}

					TArray<FText> warningMessages;
					TArray<FName> warningNames;

					bool bBuildSuccess = 
						MeshUtilities.BuildSkeletalMesh( 
							model->LODModels[0],
							skeltalMesh->RefSkeleton, 
							lodInfluences, lodWedges, lodFaces, lodPoints, lodPointToRawMap, 
							buildOptions,
							&warningMessages, &warningNames
						);
					if( bBuildSuccess )
					{
						skeltalMesh->PostEditChange();
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


ULiveLinkExtendInstance::ULiveLinkExtendInstance( const FObjectInitializer& Initializer )
	: Super( Initializer )
{

}

FAnimInstanceProxy* ULiveLinkExtendInstance::CreateAnimInstanceProxy()
{
	return new FLiveLinkExtendInstanceProxy( this );
}




class FLiveLinkExtendCameraController 
	: public FEditorCameraController
{
	FLiveLinkClientReference ClientRef;

public:

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


