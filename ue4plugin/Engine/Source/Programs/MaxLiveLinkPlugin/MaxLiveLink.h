
/*! @file		MaxLiveLink.h
	@brief		プラグインの動作を行う実装
*/

#if !defined( __MaxLiveLink__ )
	#define __MaxLiveLink__


	#include	<CommonInclude.h>

	class FMaxLiveLink;
	class FMaxLiveLinkCameraStreamedSubject;
	class FMaxLiveLinkSkinMeshesStreamedSubject;




	/*------- ↓ ノード更新コールバック ↓ ------- {{{ */
	
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

	/*------- ↑ ノード更新コールバック ↑ ------- }}} */




	/*------- ↓ プラグイン挙動と汎用処理 ↓ ------- {{{ */

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

			/*------- ↓ UI操作 ↓ ------- {{{ */
		
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
			//! Utilityプラグインとしての使用を開始
			virtual void	BeginEditParams( Interface *ip, IUtil *iu );
			//! Utilityプラグインとしての使用を終了
			virtual void	EndEditParams( Interface *ip, IUtil *iu );
			//! Utilityプラグインとしての削除の呼び出し
			virtual void	DeleteThis() {}
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! カスタムコントロールダイアログのWindowsプロセス
			INT_PTR		ReceiveCustomControlDialogProcess(
				UINT message, WPARAM wParam, LPARAM lParam
			);
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! ダイアログ内容の初期化
			void	InitializeDialog( HWND hWnd );
			//! ダイアログ内容の解放
			void	FinalizeDialog();
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! +Xを正面とさせるか
			bool	UseForceFrontAxisX() const;
			//! 自動でのメッシュ同期を使用するか
			bool	UseAutomaticSyncMesh() const;
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! 単位のスケールを取得
			float	GetUnitScale() const;
			//! 使用するUVの数を取得
			int		GetNumUseUVs() const;
			//! 使用するカラーの数を取得
			int		GetNumUseColors() const;
			//----------------------------------------------------------


			//----------------------------------------------------------
			BindPoseFrameType		GetBindPoseFrameType() const;

			int						GetBindPoseFrame() const;
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! ダイアログコールバック命令の受け取り
			void	OnReceiveDialogCallbackCommand( WPARAM wParam );
			//! テキストエディットのコールバック命令の受け取り
			void	OnReceiveEnterEditCallbackCommand( WPARAM wParam );
			//----------------------------------------------------------

			/*------- ↑ UI操作 ↑ ------- }}} */



			/*------- ↓ Subject管理 ↓ ------- {{{ */

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
			//! Subject用ノードの収集を再帰的に行う
			void	CollectSubjectNodes_Recursive(
				NodeArray& outArray, INode* currentNode,
				uint32 collectOption
			);
			//----------------------------------------------------------

		public:



			//----------------------------------------------------------
			//! Subject用ノードの収集
			NodeArray	CollectSubjectNodes(
				uint32 collectOption = CollectOptionBits_AllTypes
			);
			//! Subject用ノードの収集
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
			//! 編集されるSubjectの追加　※追加されるSubjectはUIの状態にて設定される
			void	AddEditedSubject();
			//! 選択されているSubjectの削除
			void	RemoveSelectedSubject();

			//! すべてのSkinのSubjectを削除
			void	DeleteAllSkinNodeSubject();
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! 更新されたジオメトリーのノードを受け取り
			void	ReceiveChangeGeometoryNodes( const TArray<INode*>& nodes );

			//! ノードがSubjectのどれかによって保持されているか
			bool	IsContainNodesFromAnySubjects( const TArray<INode*>& nodes ) const;

			//! すべてスキンSubjectのメッシュ同期の最終更新時のキャッシュをクリア
			void	ClearAllSkinSubjectSyncDataCache();
			//! すべてスキンSubjectのジオメトリーの更新状態を設定
			void	SettingAllSkinSubjectUpdateGeometry( bool doUpdate );
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! すべてのSubjectのノードリストの内容が存在しているかを確認して参照状態を更新
			void	UpdateAllSubjectNodeListReference();
			//----------------------------------------------------------

			/*------- ↑ Subject管理 ↑ ------- }}} */



			/*------- ↓ 更新受け取りと送信 ↓ ------- {{{ */

		private:
			TimeValue	lastUpdateTime_;
			int			lastViewID_;

		public:

			//----------------------------------------------------------
			//! バインドポーズ用の時間を取得
			TimeValue	GetBindPoseTime() const;
			//----------------------------------------------------------

			//----------------------------------------------------------
			//! 描画の受け取り
			void	OnDisplay( TimeValue t, ViewExp *vpt, int flags );
			//! 現在のフレームのSubjectの状態を適応
			void	ApplyCurrentFrameSubjects( bool forceUpdateMesh = false );
			//----------------------------------------------------------

			/*------- ↑ 更新受け取りと送信 ↑ ------- }}} */



			/*------- ↓ 汎用行列取得 ↓ ------- {{{ */
			
		public:

			static const Matrix3	FrontXMatrix;
			static const Matrix3&	GetFrontXMatrix();

			static const Matrix3	InvertFrontZMatrix;
			static const Matrix3&	GetInvertFrontZMatrix();


			static const Matrix3	FrontXRootMatrix;
			static const Matrix3&	GetFrontXRootMatrix();

			/*------- ↑ 汎用行列取得 ↑ ------- }}} */
		};

	/*------- ↑ プラグイン挙動と汎用処理 ↑ ------- }}} */

#endif// __MaxLiveLink__


