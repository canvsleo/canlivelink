
/*! @file		SkinMeshesStreamedSubject.h
	@brief		�X�L�����b�V�������p��Subject���Ǘ�����\��
*/

#pragma once

#if !defined( __SkinMeshesStreamedSubject__ )
	#define __SkinMeshesStreamedSubject__


	#include	"BaseStreamedSubject.h"

	#include	"../LiveLinkExtendVertexMeshData.h"

	/*! @class		FMaxLiveLinkSkinMeshesStreamedSubject
		@brief		�X�L�����b�V�������p��Subject���Ǘ�����\��
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
		//! �X�L�����f�t�@�C�A�̃��X�g���擾
		TArray<ISkin*>		GetSkinList() const;
		//----------------------------------------------------------

		//----------------------------------------------------------
		//! ���b�V�������̍ŏI�X�V���̃L���b�V�����N���A
		void	ClearSyncDataCache();
		//----------------------------------------------------------

		//----------------------------------------------------------
		//! �X�V���ꂽ�W�I���g���[�̃m�[�h���󂯎��
		void	ReceiveUpdateGeometryOfNodes( const TArray<INode*>& nodes );
		//! �W�I���g���[�̍X�V��Ԃ�ݒ�
		void	SettingUpdateGeometry( bool doUpdate );
		//----------------------------------------------------------




	public:
		//----------------------------------------------------------
		//! Subject�Ƃ��Ẵf�[�^���\�z
		virtual void BuildSubjectData();
		//! UE4�ւ̃f�[�^���M
		virtual void OnStream( double streamTime, int32 frameNumber );
		//----------------------------------------------------------

		


	
	
	};

#endif // __SkinMeshesStreamedSubject__

