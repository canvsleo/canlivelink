
/*! @file		MaxLiveLink.h
	@brief		�v���O�C���̓�����s������
*/

#if !defined( __MaxLiveLink__ )
	#define __MaxLiveLink__


	#include	<CommonInclude.h>

	class FMaxLiveLink;
	class FMaxLiveLinkCameraStreamedSubject;
	class FMaxLiveLinkSkinMeshesStreamedSubject;




	/*------- �� �m�[�h�X�V�R�[���o�b�N �� ------- {{{ */
	
		applyable_class_debug_ok ( NodeEventCallbackValue )
		class NodeEventCallbackValue 
			: public INodeEventCallback, ViewportDisplayCallback
		{
		public:
			NodeEventCallbackValue(
				FMaxLiveLink* maxLink,
				Interface* ip
			);
			virtual ~NodeEventCallbackValue();

		private:
			FMaxLiveLink*		maxLink_;
			Interface*			interface_;

			NodeEventNamespace::CallbackKey nodeEventKey_;

		public:
			virtual void ModelStructured( NodeKeyTab& nodes );
			virtual void GeometryChanged( NodeKeyTab& nodes );
			virtual void MappingChanged( NodeKeyTab& nodes );

			virtual void NameChanged( NodeKeyTab& nodes );

			virtual void Added( NodeKeyTab& nodes );
			virtual void Deleted( NodeKeyTab& nodes );
			virtual void LinkChanged( NodeKeyTab& nodes );
			virtual void LayerChanged( NodeKeyTab& nodes );
			virtual void GroupChanged( NodeKeyTab& nodes );
			virtual void HierarchyOtherEvent( NodeKeyTab& nodes );

			virtual void Display( TimeValue t, ViewExp *vpt, int flags );
			virtual void GetViewportRect( TimeValue t, ViewExp* vpt, Rect* rect );
			virtual BOOL Foreground();
		};

	/*------- �� �m�[�h�X�V�R�[���o�b�N �� ------- }}} */




	/*------- �� �v���O�C�������Ɣėp���� �� ------- {{{ */

		class FMaxLiveLink
			: public UtilityObj
		{
			friend class CCUtilClassDesc;

		public:
			FMaxLiveLink(
				HINSTANCE instanceHandle
			);
			~FMaxLiveLink();

			

			void Init( HWND hWnd );
			void Destroy( HWND hWnd );

		private:

			HINSTANCE	instanceHandle_;
			HWND		controlPageHandle_;
			NodeEventCallbackValue* nodeEvent_;

			Interface*	interface_;

		private:
			TSharedPtr<ILiveLinkProvider>	liveLinkProvider_;
			FDelegateHandle					connectionStatusChangedHandle_;

		public:
			ILiveLinkProvider*	GetLiveLinkProvider() const { return this->liveLinkProvider_.Get(); }

			/*------- �� UI���� �� ------- {{{ */
		
		public:
			typedef enum _BindPoseFrameType_
			{
				BindPoseFrameType_FirstFrame	= 0,
				BindPoseFrameType_SelectFrame	= 1
			}BindPoseFrameType;


		private:
			HWND				windowHandle_;
			FMaxLiveLinkCameraStreamedSubject*			editViewCameraSubject_;


			ICustButton*		buttonAddSubject_;
			ICustButton*		buttonRemoveSubject_;

			ICustEdit*			textSubjectName_;
			HWND				listSubject_;
				
			bool useForceFrontX_Cache_;
			bool useAutomaticSyncMesh_Cache_;

			ISpinnerControl*	numUnitScale_;
				float				unitScale_Cache_;

			ICustButton*		buttonSendMesh_;

			ISpinnerControl*	numVertexUVs_;
				int					vertexUVs_Cache_;
			ISpinnerControl*	numVertexColors_;
				int					vertexColors_Cache_;

			BindPoseFrameType	bindPoseFrameType_;

			ISpinnerControl*	numBindPoseFrame_;
				int					numBindPoseFrame_Cache_;
			

		public:

			//----------------------------------------------------------
			//! Utility�v���O�C���Ƃ��Ă̎g�p���J�n
			virtual void	BeginEditParams( Interface *ip, IUtil *iu );
			//! Utility�v���O�C���Ƃ��Ă̎g�p���I��
			virtual void	EndEditParams( Interface *ip, IUtil *iu );
			//! Utility�v���O�C���Ƃ��Ă̍폜�̌Ăяo��
			virtual void	DeleteThis() {}
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! �J�X�^���R���g���[���_�C�A���O��Windows�v���Z�X
			INT_PTR		ReceiveCustomControlDialogProcess(
				UINT message, WPARAM wParam, LPARAM lParam
			);
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! �_�C�A���O���e�̏�����
			void	InitializeDialog( HWND hWnd );
			//! �_�C�A���O���e�̉��
			void	FinalizeDialog();
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! +X�𐳖ʂƂ����邩
			bool	UseForceFrontAxisX() const;
			//! �����ł̃��b�V���������g�p���邩
			bool	UseAutomaticSyncMesh() const;
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! �P�ʂ̃X�P�[�����擾
			float	GetUnitScale() const;
			//! �g�p����UV�̐����擾
			int		GetNumUseUVs() const;
			//! �g�p����J���[�̐����擾
			int		GetNumUseColors() const;
			//----------------------------------------------------------


			//----------------------------------------------------------
			BindPoseFrameType		GetBindPoseFrameType() const;

			int						GetBindPoseFrame() const;
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! �_�C�A���O�R�[���o�b�N���߂̎󂯎��
			void	OnReceiveDialogCallbackCommand( WPARAM wParam );
			//! �e�L�X�g�G�f�B�b�g�̃R�[���o�b�N���߂̎󂯎��
			void	OnReceiveEnterEditCallbackCommand( WPARAM wParam );
			//----------------------------------------------------------

			/*------- �� UI���� �� ------- }}} */



			/*------- �� Subject�Ǘ� �� ------- {{{ */

		public:
			typedef enum _CollectOptionBits_
			{
				CollectOptionBits_None				= 0,
				CollectOptionBits_OnlySelected		= 0x1 << 0,

				CollectOptionBits_Mesh				= 0x1 << 8,
				CollectOptionBits_Bone				= 0x1 << 9,
				CollectOptionBits_Camera			= 0x1 << 10,
				CollectOptionBits_Helper			= 0x1 << 11,
				CollectOptionBits_Dummy				= 0x1 << 12,

				CollectOptionBits_SkinedMesh		= 0x1 << 20,
				CollectOptionBits_MorphMesh			= 0x1 << 21,

				CollectOptionBits_AllTypes =
				CollectOptionBits_Mesh
				| CollectOptionBits_Bone
				| CollectOptionBits_Camera
				| CollectOptionBits_Helper
				| CollectOptionBits_Dummy
				,

				CollectOptionBits_AllNode = 0xFFFFFFFF

			}CollectOptionBits;

		private:

			//----------------------------------------------------------
			//! Subject�p�m�[�h�̎��W���ċA�I�ɍs��
			void	CollectSubjectNodes_Recursive(
				NodeArray& outArray, INode* currentNode,
				uint32 collectOption
			);
			//----------------------------------------------------------

		public:



			//----------------------------------------------------------
			//! Subject�p�m�[�h�̎��W
			NodeArray	CollectSubjectNodes(
				uint32 collectOption = CollectOptionBits_AllTypes
			);
			//! Subject�p�m�[�h�̎��W
			void	CollectSubjectNodes(
				NodeArray& outArray,
				uint32 collectOption = CollectOptionBits_AllTypes
			);
			//----------------------------------------------------------

		private:
			TMap< FString, FMaxLiveLinkSkinMeshesStreamedSubject* >	skinNodeSubjectList_;
			TArray<FMaxLiveLinkCameraStreamedSubject*>				cameraSubjectList_;

		public:

			//----------------------------------------------------------
			//! �ҏW�����Subject�̒ǉ��@���ǉ������Subject��UI�̏�ԂɂĐݒ肳���
			void	AddEditedSubject();
			//! �I������Ă���Subject�̍폜
			void	RemoveSelectedSubject();

			//! ���ׂĂ�Skin��Subject���폜
			void	DeleteAllSkinNodeSubject();
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! �X�V���ꂽ�W�I���g���[�̃m�[�h���󂯎��
			void	ReceiveChangeGeometoryNodes( const TArray<INode*>& nodes );

			//! �m�[�h��Subject�̂ǂꂩ�ɂ���ĕێ�����Ă��邩
			bool	IsContainNodesFromAnySubjects( const TArray<INode*>& nodes ) const;

			//! ���ׂăX�L��Subject�̃��b�V�������̍ŏI�X�V���̃L���b�V�����N���A
			void	ClearAllSkinSubjectSyncDataCache();
			//! ���ׂăX�L��Subject�̃W�I���g���[�̍X�V��Ԃ�ݒ�
			void	SettingAllSkinSubjectUpdateGeometry( bool doUpdate );
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! ���ׂĂ�Subject�̃m�[�h���X�g�̓��e�����݂��Ă��邩���m�F���ĎQ�Ə�Ԃ��X�V
			void	UpdateAllSubjectNodeListReference();
			//----------------------------------------------------------

			/*------- �� Subject�Ǘ� �� ------- }}} */



			/*------- �� �X�V�󂯎��Ƒ��M �� ------- {{{ */

		private:
			TimeValue	lastUpdateTime_;
			int			lastViewID_;

		public:

			//----------------------------------------------------------
			//! �o�C���h�|�[�Y�p�̎��Ԃ��擾
			TimeValue	GetBindPoseTime() const;
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! �`��̎󂯎��
			void	OnDisplay( TimeValue t, ViewExp *vpt, int flags );
			//! ���݂̃t���[����Subject�̏�Ԃ�K��
			void	ApplyCurrentFrameSubjects( bool forceUpdateMesh = false );
			//----------------------------------------------------------

			/*------- �� �X�V�󂯎��Ƒ��M �� ------- }}} */



			/*------- �� �ėp�s��擾 �� ------- {{{ */
			
		public:

			static const Matrix3	FrontXMatrix;
			static const Matrix3&	GetFrontXMatrix();

			static const Matrix3	InvertFrontZMatrix;
			static const Matrix3&	GetInvertFrontZMatrix();


			static const Matrix3	FrontXRootMatrix;
			static const Matrix3&	GetFrontXRootMatrix();

			/*------- �� �ėp�s��擾 �� ------- }}} */
		};

	/*------- �� �v���O�C�������Ɣėp���� �� ------- }}} */

#endif// __MaxLiveLink__


