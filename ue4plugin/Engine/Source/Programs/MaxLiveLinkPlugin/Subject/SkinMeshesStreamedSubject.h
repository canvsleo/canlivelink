
/*! @file		SkinMeshesStreamedSubject.h
	@brief		スキンメッシュ同期用のSubjectを管理する構成
*/

#pragma once

#if !defined( __SkinMeshesStreamedSubject__ )
	#define __SkinMeshesStreamedSubject__


	#include	"BaseStreamedSubject.h"

	#include	"../LiveLinkExtendVertexMeshData.h"

	/*! @class		FMaxLiveLinkSkinMeshesStreamedSubject
		@brief		スキンメッシュ同期用のSubjectを管理する構成
	*/
	class FMaxLiveLinkSkinMeshesStreamedSubject
		: public FMaxLiveLinkBaseStreamedSubject
	{
	public:
		FMaxLiveLinkSkinMeshesStreamedSubject(
			FMaxLiveLink* livelinkReference,
			INode* node, const FName& subjectName
		);

		FMaxLiveLinkSkinMeshesStreamedSubject(
			FMaxLiveLink* livelinkReference,
			const NodeArray& nodeList, const FName& subjectName
		);


		virtual ~FMaxLiveLinkSkinMeshesStreamedSubject();

	private:
		NodeArray	jointNodeList_;
		FLiveLinkExtendSyncData	lastSyncData_;

	public:

		//----------------------------------------------------------
		//! スキンモデファイアのリストを取得
		TArray<ISkin*>		GetSkinList() const;
		//----------------------------------------------------------

		//----------------------------------------------------------
		//! メッシュ同期の最終更新時のキャッシュをクリア
		void	ClearSyncDataCache();
		//----------------------------------------------------------

		//----------------------------------------------------------
		//! 更新されたジオメトリーのノードを受け取り
		void	ReceiveUpdateGeometryOfNodes( const TArray<INode*>& nodes );
		//! ジオメトリーの更新状態を設定
		void	SettingUpdateGeometry( bool doUpdate );
		//----------------------------------------------------------




	public:
		//----------------------------------------------------------
		//! Subjectとしてのデータを構築
		virtual void BuildSubjectData();
		//! UE4へのデータ送信
		virtual void OnStream( double streamTime, int32 frameNumber );
		//----------------------------------------------------------

		


	
	
	};

#endif // __SkinMeshesStreamedSubject__

