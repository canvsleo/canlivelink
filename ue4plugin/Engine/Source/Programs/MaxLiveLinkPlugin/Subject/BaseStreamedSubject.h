
/*! @file		BaseStreamedSubject.h
	@brief		���M����Subject���Ǘ������b�\��
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
		//! �m�[�h���X�g�̓��e�����݂��Ă��邩���m�F���ĎQ�Ə�Ԃ��X�V
		void	UpdateNodeListReference();

		//! �m�[�h�Q�Ƃ�1�ł����݂��邩
		bool	HasAnyNodeReferences() const;
		//----------------------------------------------------------

		//----------------------------------------------------------
		//! �m�[�h���ێ�����Ă��邩
		bool	IsContainNodes( const TArray<INode*>& nodes ) const;
		//----------------------------------------------------------

		//----------------------------------------------------------
		//! Subject�Ƃ��Ẵf�[�^���\�z
		virtual void	BuildSubjectData() = 0;
		//! UE4�ւ̃f�[�^���M
		virtual void	OnStream( double streamTime, int32 frameNumber ) = 0;
		//----------------------------------------------------------


		/*------- �� �g�����X�t�H�[���̕ϊ� �� ------- {{{ */
	public:


		static const Matrix3 MaxIdentityMatrix;

		//----------------------------------------------------------
		//! 3DSMax��Matrix���g�����X�t�H�[���ɕϊ�
		static FTransform	ConvertMaxMatrixToTransform( const Matrix3& mtx );
		//----------------------------------------------------------

		//----------------------------------------------------------
		//! �m�[�h����g�����X�t�H�[�����擾
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
		//! �m�[�h����g�����X�t�H�[�����擾
		static FTransform	GetSubjectTransform(
			INode* node,
			TimeValue timeValue,
			bool frontAxisX, bool forRootNode = false
		);
		//----------------------------------------------------------

		/*------- �� �g�����X�t�H�[���̕ϊ� �� ------- }}} */

	};


#endif // __BaseStreamedSubject__

