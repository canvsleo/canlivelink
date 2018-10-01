
/*! @file		SkinMeshesStreamedSubject.h
	@brief		スキンメッシュ同期用のSubjectを管理する構成
*/


#include	"SkinMeshesStreamedSubject.h"

#include	"../MaxLiveLink.h"

#include	"MorphR3.h"

#include	"Base64.h"

FMaxLiveLinkSkinMeshesStreamedSubject::FMaxLiveLinkSkinMeshesStreamedSubject(
	FMaxLiveLink* livelinkReference,
	INode* node, const FName& subjectName
)
	: FMaxLiveLinkBaseStreamedSubject(
		livelinkReference,
		node, subjectName
	)
{
	this->BuildSubjectData();
}

FMaxLiveLinkSkinMeshesStreamedSubject::FMaxLiveLinkSkinMeshesStreamedSubject(
	FMaxLiveLink* livelinkReference,
	const NodeArray& nodeList, const FName& subjectName
)
	: FMaxLiveLinkBaseStreamedSubject(
		livelinkReference,
		nodeList, subjectName
	)
{
	this->BuildSubjectData();
}


FMaxLiveLinkSkinMeshesStreamedSubject::~FMaxLiveLinkSkinMeshesStreamedSubject()
{
	this->GetLiveLinkProvider()->ClearSubject( this->subjectName_ );
}



/*! @brief		スキンモデファイアのリストを取得
*/
TArray<ISkin*>		FMaxLiveLinkSkinMeshesStreamedSubject::GetSkinList() const
{
	TArray<ISkin*> ret;

	for( auto node : this->nodeList_ )
	{
		if( !node.ReferencedNode )
		{
			continue;
		}
		auto derivedObject	= reinterpret_cast<IDerivedObject*>( node.ReferencedNode->GetObjectRef() );
		if( !derivedObject )
		{
			continue;
		}
		int numModifier = derivedObject->NumModifiers();
		for( int iModifier = 0; iModifier < numModifier; ++iModifier )
		{
			auto modifier = derivedObject->GetModifier( iModifier );
			if( modifier )
			{
				auto skin = reinterpret_cast<ISkin*>( modifier->GetInterface( I_SKIN ) );
				if( skin )
				{
					ret.Add( skin );
				}
			}
		}
	}
	return ret;
}



/*! @brief		メッシュ同期の最終更新時のキャッシュをクリア
*/
void	FMaxLiveLinkSkinMeshesStreamedSubject::ClearSyncDataCache()
{
	this->lastSyncData_ = FLiveLinkExtendSyncData();
}



/*! @brief		更新されたジオメトリーのノードを受け取り
*/
void	FMaxLiveLinkSkinMeshesStreamedSubject::ReceiveUpdateGeometryOfNodes( const TArray<INode*>& nodes )
{
	for( int iNode = 0; iNode < this->nodeList_.Num(); ++iNode )
	{
		for( int iUpdateNodes = 0; iUpdateNodes < nodes.Num(); ++iUpdateNodes )
		{
			if( this->nodeList_[ iNode ].ReferencedNode == nodes[ iUpdateNodes ] )
			{
				this->nodeList_[ iNode ].ShouldUpdateGeometry = true;
				break;
			}
		}
	}
}

/*! @brief		ジオメトリーの更新状態を設定
*/
void	FMaxLiveLinkSkinMeshesStreamedSubject::SettingUpdateGeometry( bool doUpdate )
{
	for( int iNode = 0; iNode < this->nodeList_.Num(); ++iNode )
	{
		this->nodeList_[ iNode ].ShouldUpdateGeometry = doUpdate;
	}
}





/*! @brief		Subjectとしてのデータを構築
*/
void FMaxLiveLinkSkinMeshesStreamedSubject::BuildSubjectData()
{
	TArray<FName>	jointNames;
	TArray<int32>	jointParents;
	{
		for( int iNode = 0; iNode < this->nodeList_.Num(); ++iNode )
		{
			auto& nodeReference = this->nodeList_[ iNode ];
			auto node = nodeReference.ReferencedNode;
			const auto nodeName = node->GetName();

			nodeReference.ParentIndexCache = ParentIndex_None;
			for( int jNode = 0; jNode < this->nodeList_.Num(); ++jNode )
			{
				if( node->GetParentNode() == this->nodeList_[ jNode ].ReferencedNode )
				{
					nodeReference.ParentIndexCache = jNode;
					break;
				}
			}
			jointNames.Add( nodeReference.NodeName );
			jointParents.Add( nodeReference.ParentIndexCache );
		}
	}

	this->GetLiveLinkProvider()->UpdateSubject(
		this->subjectName_,
		jointNames,
		jointParents
	);
}

/*! @brief		UE4へのデータ送信
*/
void	FMaxLiveLinkSkinMeshesStreamedSubject::OnStream( double streamTime, int32 frameNumber )
{
	TArray<FTransform> transformList;
	TArray<FLiveLinkCurveElement> curves;

	bool doMeshSync = false;

	FLiveLinkMetaData metaData;
	FLiveLinkExtendSyncData syncMeshData;

	FDateTime currentTime = FDateTime::Now();

	const auto& frontXMatrix		= FMaxLiveLink::GetFrontXMatrix();
	const auto& invFrontZMatrix		= FMaxLiveLink::GetInvertFrontZMatrix();
	const auto& frontXRootMatrix	= FMaxLiveLink::GetFrontXRootMatrix();

	{
		for( int iNode = 0; iNode < this->nodeList_.Num(); ++iNode )
		{
			auto& nodeReference = this->nodeList_[ iNode ];
			auto node = nodeReference.ReferencedNode;
			auto transform =
				GetSubjectTransformFromSeconds(
					node, streamTime,
					this->livelinkReference_->UseForceFrontAxisX(),
					nodeReference.ParentIndexCache == ParentIndex_None
				);
			if( !node )
			{
				transformList.Add( transform );
				continue;
			}

			const auto& nodeName = nodeReference.NodeName;

			transformList.Add( transform );

			if( !nodeReference.ShouldUpdateGeometry )
			{
				continue;
			}
			nodeReference.ShouldUpdateGeometry = false;

			auto obj	= node->GetObjectRef();
			if( !obj )
			{
				continue;
			}

			

			auto material = node->GetMtl();

			ISkin*		skinObject	= nullptr;
			MorphR3*	morphObject	= nullptr;
			{
				auto deriverdObj	= reinterpret_cast<IDerivedObject*>( obj );
				int numModifier = deriverdObj->NumModifiers();
				for( int iModifier = 0; iModifier < numModifier; ++iModifier )
				{
					auto modifier = deriverdObj->GetModifier( iModifier );
					if( modifier )
					{

						auto skin = reinterpret_cast<ISkin*>( modifier->GetInterface( I_SKIN ) );
						if( skin )
						{
							skinObject = skin;
							ObjectState state = obj->Eval( this->livelinkReference_->GetBindPoseTime() );
							obj = state.obj;
							break;
						}

						if(
							modifier->SuperClassID() == OSM_CLASS_ID &&
							modifier->ClassID() == MORPHER_CLASS_ID
							)
						{
							morphObject = static_cast<MorphR3*>( modifier );

							ObjectState state = obj->Eval( this->livelinkReference_->GetBindPoseTime() );
							obj = state.obj;

							break;
						}
					}
				}
			}


			FLiveLinkExtendVertexMeshData meshData;

			auto id		= obj->ClassID();
			auto sid	= obj->SuperClassID();

			TriObject* triObject	= nullptr;
			PolyObject* polyObject	= nullptr;
			if( obj->IsSubClassOf( triObjectClassID ) )
			{
				triObject = ( TriObject* )obj;
			}
			else if(
				sid == GEOMOBJECT_CLASS_ID &&
				( id == EPOLYOBJ_CLASS_ID || id.PartA() == POLYOBJ_CLASS_ID )
				)
			{
				polyObject = ( PolyObject* )obj;
			}
			else if(
				sid == GEOMOBJECT_CLASS_ID &&
				( id.PartA() == EDITTRIOBJ_CLASS_ID || id.PartA() == TRIOBJ_CLASS_ID )
				)
			{
				triObject = ( TriObject* )obj;
			}
			else if( obj->CanConvertToType( Class_ID( POLYOBJ_CLASS_ID, 0 ) ) )
			{
				polyObject = ( PolyObject* )obj;
			}
			else if( obj->CanConvertToType( Class_ID( TRIOBJ_CLASS_ID, 0 ) ) )
			{
				triObject = ( TriObject* )obj;
			}


			bool hasNodeMeshData = false;

			if( triObject )
			{
				int numVertexies	= triObject->mesh.numVerts;
				int numFaces		= triObject->mesh.numFaces;
				if( numVertexies > 0 && numFaces > 0 )
				{
					meshData.MeshName	= nodeName.ToString();
					meshData.TimeStamp	= currentTime;

					meshData.NumUseUVs		= this->livelinkReference_->GetNumUseUVs();
					meshData.NumUseColors	= this->livelinkReference_->GetNumUseColors();

					auto bindPoseMatrix = node->GetObjTMAfterWSM( this->livelinkReference_->GetBindPoseTime(), nullptr );
					bindPoseMatrix.NoScale();
					if( this->livelinkReference_->UseForceFrontAxisX() )
					{
						bindPoseMatrix = ( bindPoseMatrix * invFrontZMatrix ) * frontXMatrix;
					}

					{
						for( int iVertex = 0; iVertex < numVertexies; ++iVertex )
						{
							auto vertexPos = triObject->mesh.getVertPtr( iVertex );
							auto resultPos = bindPoseMatrix * *vertexPos;

							resultPos.y = -resultPos.y;

							meshData.PointList.Add(
								FLiveLinkExtendPoint3(
									resultPos.x,
									resultPos.y,
									resultPos.z
								)
							);
						}
					}

					const int NotHasNormalSpec		= 0;
					const int HasVertexNormalSpec	= 1;
					const int HasFacesNormalSpec	= 2;

					int hasNormalSpec = NotHasNormalSpec;

					MeshNormalSpec* normalSpec = nullptr;
					normalSpec = triObject->mesh.GetSpecifiedNormals();
					if( normalSpec )
					{
						if( numVertexies == normalSpec->GetNumNormals() )
						{
							hasNormalSpec = HasVertexNormalSpec;
						}
						else if( numFaces == normalSpec->GetNumFaces() )
						{
							hasNormalSpec = HasFacesNormalSpec;
						}
					}

					if( hasNormalSpec == HasVertexNormalSpec )
					{
						for( int iNormal = 0; iNormal < numVertexies; ++iNormal )
						{
							const auto& normal = normalSpec->Normal( iNormal );
							auto resultVec = bindPoseMatrix * normal;

							resultVec.y = -resultVec.y;
							meshData.PointNormalList.Add(
								FLiveLinkExtendPoint3( resultVec.x, resultVec.y, resultVec.z )
							);

						}
					}
					else if( hasNormalSpec == NotHasNormalSpec )
					{
						for( int iVertex = 0; iVertex < numVertexies; ++iVertex )
						{
							auto normal = triObject->mesh.getNormal( iVertex );
							auto resultVec = bindPoseMatrix * normal;

							resultVec.y = -resultVec.y;
							meshData.PointNormalList.Add(
								FLiveLinkExtendPoint3( resultVec.x, resultVec.y, resultVec.z )
							);
						}

					}


					int maxTextureCoord = triObject->mesh.getNumMaps();
					TMap<MtlID, TArray<int> > materialFaceMap;
					{
						for( int iFace = 0; iFace < numFaces; ++iFace )
						{
							auto materialID		= triObject->mesh.getFaceMtlIndex( iFace );
							auto findMaterial	= materialFaceMap.Find( materialID );
							if( findMaterial )
							{
								findMaterial->Add( iFace );
							}
							else
							{
								TArray<int> faceArray;
								faceArray.Push( iFace );
								materialFaceMap.Add( materialID, faceArray );
							}
						}
					}


					for( const auto& facePair : materialFaceMap )
					{
						if( facePair.Key >= material->NumSubMtls() )
						{
							continue;
						}

						auto groupMaterial = material->GetSubMtl( facePair.Key );

						FLiveLinkExtendMaterialGroupMeshData groupData;

						groupData.Triangles		= 0;
						if( materialFaceMap.Num() == 1 )
						{
							groupData.MaterialName	= material->GetName();
						}
						else
						{
							groupData.MaterialName	= groupMaterial->GetName();
						}

						const auto& faceList = facePair.Value;

						const int numTriangle = 3;
						int numMaterialFaces = facePair.Value.Num();
						for( int iMaterialFace = 0; iMaterialFace < numMaterialFaces; ++iMaterialFace )
						{
							auto faceIndex		= faceList[ iMaterialFace ];
							const auto& face	= triObject->mesh.faces[ faceIndex ];

							FLiveLinkExtendPolygon polygon;
							polygon.SmoothingGroup = face.smGroup;

							for( int iVertexIndex = 0; iVertexIndex < 3; ++iVertexIndex )
							{
								auto vertexIndex = face.v[ iVertexIndex ];
								polygon.VertexIndexList.Push( vertexIndex );
							}

							if( hasNormalSpec == HasFacesNormalSpec )
							{
								for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
								{
									const auto& normal = normalSpec->GetNormal( iMaterialFace, iTriangle );
									auto resultVec = bindPoseMatrix * normal;

									resultVec.y = -resultVec.y;
									polygon.NormalList.Add(
										FLiveLinkExtendPoint3( resultVec.x, resultVec.y, resultVec.z )
									);
								}

							}

							polygon.ColorList0.Init( FLiveLinkExtendPoint4( 1.0f, 1.0f, 1.0f, 1.0f ), numTriangle );

							for( int iMapChannel = -NUM_HIDDENMAPS; iMapChannel < triObject->mesh.numMaps; ++iMapChannel )
							{
								auto& mapChannel = triObject->mesh.Map( iMapChannel );
								if( !mapChannel.GetFlag( MN_DEAD ) && iMaterialFace < mapChannel.fnum )
								{
									const auto& mapFace = mapChannel.tf[ iMaterialFace ];
									switch( iMapChannel )
									{
									case MAP_ALPHA:
										{
											if( meshData.NumUseColors >= 1 )
											{
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& color = mapChannel.tv[ vIndex ];
													polygon.ColorList0[ iTriangle ].W = color.x;
												}
											}
										}break;
									case 0:
										{
											if( meshData.NumUseColors >= 1 )
											{
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& color = mapChannel.tv[ vIndex ];
													polygon.ColorList0[ iTriangle ].X = color.x;
													polygon.ColorList0[ iTriangle ].Y = color.y;
													polygon.ColorList0[ iTriangle ].Z = color.z;
												}
											}
										}break;

									case 1:
										{
											if( meshData.NumUseUVs >= 1 )
											{
												polygon.UVList0.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& uv = mapChannel.tv[ vIndex ];
													polygon.UVList0[ iTriangle ].X = uv.x;
													polygon.UVList0[ iTriangle ].Y = uv.y;
												}
											}
										}break;
									case 2:
										{
											if( meshData.NumUseUVs >= 2 )
											{
												polygon.UVList1.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& uv = mapChannel.tv[ vIndex ];
													polygon.UVList1[ iTriangle ].X = uv.x;
													polygon.UVList1[ iTriangle ].Y = uv.y;
												}
											}
										}break;
									case 3:
										{
											if( meshData.NumUseUVs >= 3 )
											{
												polygon.UVList2.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& uv = mapChannel.tv[ vIndex ];
													polygon.UVList2[ iTriangle ].X = uv.x;
													polygon.UVList2[ iTriangle ].Y = uv.y;
												}
											}
										}break;
									case 4:
										{
											if( meshData.NumUseUVs >= 4 )
											{
												polygon.UVList3.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
												for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
												{
													auto vIndex = mapFace.t[ iTriangle ];
													const auto& uv = mapChannel.tv[ vIndex ];
													polygon.UVList3[ iTriangle ].X = uv.x;
													polygon.UVList3[ iTriangle ].Y = uv.y;
												}
											}
										}break;

									default:
										break;

									}
								}
							}

							groupData.PolygonList.Push( polygon );
						}
						meshData.MaterialGroupList.Push( groupData );
					}

					hasNodeMeshData = true;
				}
			}
			else if( polyObject )
			{
				meshData.MeshName	= nodeName.ToString();
				meshData.TimeStamp	= currentTime;

				meshData.NumUseUVs		= this->livelinkReference_->GetNumUseUVs();
				meshData.NumUseColors	= this->livelinkReference_->GetNumUseColors();

				int numVertexies	= polyObject->mm.numv;
				int numFaces		= polyObject->mm.numf;
				if( numVertexies > 0 && numFaces > 0 )
				{

					auto bindPoseMatrix = node->GetObjTMAfterWSM( this->livelinkReference_->GetBindPoseTime(), nullptr );
					bindPoseMatrix.NoScale();
					if( this->livelinkReference_->UseForceFrontAxisX() )
					{
						bindPoseMatrix = ( bindPoseMatrix * invFrontZMatrix ) * frontXMatrix;
					}

					for( int iVertex = 0; iVertex < numVertexies; ++iVertex )
					{
						MNVert* vertex = polyObject->mm.V( iVertex );
						auto resultPos = bindPoseMatrix * vertex->p;

						resultPos.y = -resultPos.y;

						meshData.PointList.Add(
							FLiveLinkExtendPoint3(
								resultPos.x,
								resultPos.y,
								resultPos.z
							)
						);
					}

					const int NotHasNormalSpec		= 0;
					const int HasVertexNormalSpec	= 1;
					const int HasFacesNormalSpec	= 2;

					int hasNormalSpec = NotHasNormalSpec;

					MNNormalSpec* normalSpec = nullptr;
					normalSpec = polyObject->mm.GetSpecifiedNormals();
					if( normalSpec )
					{
						if( numVertexies == normalSpec->GetNumNormals() )
						{
							hasNormalSpec = HasVertexNormalSpec;
						}
						else if( numFaces == normalSpec->GetNumFaces() )
						{
							hasNormalSpec = HasFacesNormalSpec;
						}
					}

					if( hasNormalSpec == HasVertexNormalSpec )
					{
						for( int iNormal = 0; iNormal < numVertexies; ++iNormal )
						{
							const auto& normal = normalSpec->Normal( iNormal );
							auto resultVec = bindPoseMatrix * normal;

							resultVec.y = -resultVec.y;
							meshData.PointNormalList.Add(
								FLiveLinkExtendPoint3( resultVec.x, resultVec.y, resultVec.z )
							);
						}
					}
					else if( hasNormalSpec == NotHasNormalSpec )
					{
						for( int iVertex = 0; iVertex < numVertexies; ++iVertex )
						{
							auto normal = polyObject->mm.GetVertexNormal( iVertex );
							auto resultVec = bindPoseMatrix * normal;

							resultVec.y = -resultVec.y;
							meshData.PointNormalList.Add(
								FLiveLinkExtendPoint3( resultVec.x, resultVec.y, resultVec.z )
							);
						}

					}


					TMap<MtlID, TArray<int> > materialFaceMap;
					{

						for( int iFace = 0; iFace < numFaces; ++iFace )
						{
							auto face = polyObject->mm.F( iFace );
							if( face )
							{
								auto findMaterial = materialFaceMap.Find( face->material );
								if( findMaterial )
								{
									findMaterial->Add( iFace );
								}
								else
								{
									TArray<int> faceArray;
									faceArray.Push( iFace );
									materialFaceMap.Add( face->material, faceArray );
								}
							}
						}
					}

					for( const auto& facePair : materialFaceMap )
					{
						if( facePair.Key >= material->NumSubMtls() )
						{
							continue;
						}

						auto groupMaterial = material->GetSubMtl( facePair.Key );

						FLiveLinkExtendMaterialGroupMeshData groupData;
						groupData.Triangles	= 0;
						if( materialFaceMap.Num() == 1 )
						{
							groupData.MaterialName	= material->GetName();
						}
						else
						{
							groupData.MaterialName	= groupMaterial->GetName();
						}



						int numMaterialFaces	= polyObject->mm.numf;
						for( int iMaterialFace = 0; iMaterialFace < numMaterialFaces; ++iMaterialFace )
						{
							auto face = polyObject->mm.F( iMaterialFace );
							if( face )
							{
								Tab<int> triangleList;

								if( 0 >= face->TriNum() )
								{
									continue;
								}

								face->GetTriangles( triangleList );
								FLiveLinkExtendPolygon polygon;
								polygon.SmoothingGroup = face->smGroup;

								groupData.Triangles += face->TriNum();

								int numTriangle = triangleList.Count();
								for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
								{
									int vertexIndex = face->vtx[ triangleList[ iTriangle ] ];
									polygon.VertexIndexList.Push( vertexIndex );
								}

								if( hasNormalSpec == HasFacesNormalSpec )
								{
									for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
									{
										const auto& normal = normalSpec->GetNormal( iMaterialFace, triangleList[ iTriangle ] );
										auto resultVec = bindPoseMatrix * normal;

										resultVec.y = -resultVec.y;
										polygon.NormalList.Add(
											FLiveLinkExtendPoint3( resultVec.x, resultVec.y, resultVec.z )
										);
									}

								}

								polygon.ColorList0.Init( FLiveLinkExtendPoint4( 1.0f, 1.0f, 1.0f, 1.0f ), numTriangle );

								for( int iMapChannel = -NUM_HIDDENMAPS; iMapChannel < polyObject->mm.numm; ++iMapChannel )
								{
									const auto mapChannel = polyObject->mm.M( iMapChannel );
									if( mapChannel )
									{
										if( iMaterialFace >= mapChannel->numf )
										{
											continue;
										}

										if( !mapChannel->GetFlag( MN_DEAD ) && iMaterialFace < mapChannel->numf )
										{
											const auto& mapFace = mapChannel->f[ iMaterialFace ];
											switch( iMapChannel )
											{
											case MAP_ALPHA:
												{
													if( meshData.NumUseColors >= 1 )
													{
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& color = mapChannel->v[ vIndex ];
															polygon.ColorList0[ iTriangle ].W = color.x;
														}
													}
												}break;
											case 0:
												{
													if( meshData.NumUseColors >= 1 )
													{
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& color = mapChannel->v[ vIndex ];
															polygon.ColorList0[ iTriangle ].X = color.x;
															polygon.ColorList0[ iTriangle ].Y = color.y;
															polygon.ColorList0[ iTriangle ].Z = color.z;
														}
													}
												}break;
											case 1:
												{
													if( meshData.NumUseUVs >= 1 )
													{
														polygon.UVList0.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& uv = mapChannel->v[ vIndex ];
															polygon.UVList0[ iTriangle ].X = uv.x;
															polygon.UVList0[ iTriangle ].Y = uv.y;
														}
													}
												}break;
											case 2:
												{
													if( meshData.NumUseUVs >= 2 )
													{
														polygon.UVList1.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& uv = mapChannel->v[ vIndex ];
															polygon.UVList1[ iTriangle ].X = uv.x;
															polygon.UVList1[ iTriangle ].Y = uv.y;
														}
													}
												}break;
											case 3:
												{
													if( meshData.NumUseUVs >= 3 )
													{
														polygon.UVList2.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& uv = mapChannel->v[ vIndex ];
															polygon.UVList2[ iTriangle ].X = uv.x;
															polygon.UVList2[ iTriangle ].Y = uv.y;
														}
													}
												}break;
											case 4:
												{
													if( meshData.NumUseUVs >= 4 )
													{
														polygon.UVList3.Init( FLiveLinkExtendPoint2( 0.0f, 0.0f ), numTriangle );
														for( int iTriangle = 0; iTriangle < numTriangle; ++iTriangle )
														{
															auto vIndex = mapFace.tv[ triangleList[ iTriangle ] ];
															const auto& uv = mapChannel->v[ vIndex ];
															polygon.UVList3[ iTriangle ].X = uv.x;
															polygon.UVList3[ iTriangle ].Y = uv.y;
														}
													}
												}break;

											default:
												{
												}break;
											}
										}
									}
								}
								groupData.PolygonList.Push( polygon );
							}
						}
						meshData.MaterialGroupList.Push( groupData );
					}
					hasNodeMeshData = true;
				}
			}

			if( hasNodeMeshData )
			{
				if( skinObject )
				{
					auto skinContext = skinObject->GetContextInterface( node );
					if( skinContext )
					{
						auto numBones = skinObject->GetNumBones();

						auto numVertex = meshData.PointList.Num();
						for( int iBone = 0; iBone < numBones; ++iBone )
						{
							auto bone = skinObject->GetBone( iBone );
							meshData.SkinNameList.Add( bone->GetName() );
						}

						for( int iVertex = 0; iVertex < numVertex; ++iVertex )
						{
							int assignedBones = skinContext->GetNumAssignedBones( iVertex );
							FLiveLinkPointSkin pointSkin;

							for( int iAssignedBone = 0; iAssignedBone < assignedBones; ++iAssignedBone )
							{
								auto weight = skinContext->GetBoneWeight( iVertex, iAssignedBone );
								if( weight < 0.1f )
								{
									continue;
								}

								FLiveLinkSkinLink link;
								link.SkinWeight	= weight;
								link.SkinIndex	= skinContext->GetAssignedBone( iVertex, iAssignedBone );
								pointSkin.SkinList.Add( link );
							}
							meshData.PointSkinList.Add( pointSkin );
						}
					}
				}

				if( morphObject )
				{
					if( morphObject->chanBank.size() > 0 )
					{
						auto bindPoseMatrix = node->GetObjTMAfterWSM( this->livelinkReference_->GetBindPoseTime(), nullptr );
						bindPoseMatrix.NoScale();
						if( this->livelinkReference_->UseForceFrontAxisX() )
						{
							bindPoseMatrix = ( bindPoseMatrix * invFrontZMatrix ) * frontXMatrix;
						}

						for( const auto& channel : morphObject->chanBank )
						{

							auto name = channel.mName;

							auto numDeltas	= channel.mDeltas.size();

							if(
								name &&
								numDeltas > 0 &&
								wcslen( name ) > 0
								)
							{

								FLiveLinkExtendMeshMorphData morphData;
								morphData.MorphName = name;
								for( int iPoint = 0; iPoint < numDeltas; ++iPoint )
								{
									FLiveLinkExtendMeshMorphVertexData vertex;

									const auto& point	= channel.mDeltas[ iPoint ];
									auto resultPos = bindPoseMatrix * point;
									resultPos.y = -resultPos.y;

									resultPos *= channel.mTargetPercent;

									vertex.DeltaPoint =
										FLiveLinkExtendPoint3(
											resultPos.x, resultPos.y, resultPos.z
										);

									morphData.VertexList.Add( vertex );
								}
								meshData.MorphList.Add( morphData );
							}
						}
					}
				}

				syncMeshData.MeshList.Add( meshData );
				doMeshSync = true;
			}

		}
	}

	if( doMeshSync )
	{
		if( this->lastSyncData_ != syncMeshData )
		{
			TArray<uint8> buffer;
			FMemoryWriter writer( buffer );

			syncMeshData << writer;


			metaData.StringMetaData.Add( "mesh_sync_data", FBase64::Encode( buffer ) );
			metaData.StringMetaData.Add( "send_time", currentTime.ToIso8601() );

			this->lastSyncData_ = syncMeshData;
		}
	}

	this->GetLiveLinkProvider()->UpdateSubjectFrame(
		this->subjectName_,
		transformList,
		curves,
		metaData,
		streamTime
	);
}

