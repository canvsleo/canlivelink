
/*! @file		BaseStreamedSubject.h
	@brief		送信するSubjectを管理する基礎構成
*/


#pragma once

#if !defined( __BaseStreamedSubject__ )
	#define __BaseStreamedSubject__


	#include	<CommonInclude.h>

	class FMaxLiveLink;


	class FMaxLiveLinkBaseStreamedSubject
	{

	public:
		FMaxLiveLinkBaseStreamedSubject(
			FMaxLiveLink* livelinkReference,
			INode* node, const FName& subjectName
		);

		FMaxLiveLinkBaseStreamedSubject(
			FMaxLiveLink* livelinkReference,
			const NodeArray& nodeList, const FName& subjectName
		);

		FMaxLiveLinkBaseStreamedSubject(
			FMaxLiveLink* livelinkReference,
			const FName& subjectName
		);

		virtual ~FMaxLiveLinkBaseStreamedSubject();

	protected:

		FMaxLiveLink*	livelinkReference_;
		ILiveLinkProvider* GetLiveLinkProvider() const;


	protected:

		static const int ParentIndex_None = -1;
		struct NodeReference
		{
			NodeReference()
				: ReferencedNode( nullptr )
				, ShouldUpdateGeometry( false )
				, ParentIndexCache( ParentIndex_None )
			{}
			NodeReference(
				INode*		node,
				bool		shouldUpdateGeometry = true
			)
				: ReferencedNode( node )
				, NodeName( node ->GetName() )
				, ShouldUpdateGeometry( shouldUpdateGeometry )
				, ParentIndexCache( ParentIndex_None )
			{}

			INode*		ReferencedNode;

			FName		NodeName;
			bool		ShouldUpdateGeometry;
			int			ParentIndexCache;
		};

		TArray<NodeReference>	nodeList_;
		FName					subjectName_;


	public:

		//----------------------------------------------------------
		//! ノードリストの内容が存在しているかを確認して参照状態を更新
		void	UpdateNodeListReference();

		//! ノード参照が1つでも存在するか
		bool	HasAnyNodeReferences() const;
		//----------------------------------------------------------

		//----------------------------------------------------------
		//! ノードが保持されているか
		bool	IsContainNodes( const TArray<INode*>& nodes ) const;
		//----------------------------------------------------------

		//----------------------------------------------------------
		//! Subjectとしてのデータを構築
		virtual void	BuildSubjectData() = 0;
		//! UE4へのデータ送信
		virtual void	OnStream( double streamTime, int32 frameNumber ) = 0;
		//----------------------------------------------------------


		/*------- ↓ トランスフォームの変換 ↓ ------- {{{ */
	public:


		static const Matrix3 MaxIdentityMatrix;

		//----------------------------------------------------------
		//! 3DSMaxのMatrixをトランスフォームに変換
		static FTransform	ConvertMaxMatrixToTransform( const Matrix3& mtx );
		//----------------------------------------------------------

		//----------------------------------------------------------
		//! ノードからトランスフォームを取得
		static FTransform	GetSubjectTransformFromSeconds(
			INode* node,
			double streamTime,
			bool frontAxisX, bool forRootNode = false
		)
		{
			return GetSubjectTransform(
				node,
				SecToTicks( streamTime ),
				frontAxisX, forRootNode
			);
		}
		//! ノードからトランスフォームを取得
		static FTransform	GetSubjectTransform(
			INode* node,
			TimeValue timeValue,
			bool frontAxisX, bool forRootNode = false
		);
		//----------------------------------------------------------

		/*------- ↑ トランスフォームの変換 ↑ ------- }}} */

	};


#endif // __BaseStreamedSubject__

