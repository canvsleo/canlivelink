
/*! @file		MaxLiveLink.cpp
	@brief		プラグインの動作を行う実装
*/

#include	"MaxLiveLink.h"

#include	"Subject/CameraStreamedSubject.h"
#include	"Subject/SkinMeshesStreamedSubject.h"

#include	"MorphR3.h"


#include	"resource.h"





/*------- ↓ ノード更新コールバック ↓ ------- {{{ */

	NodeEventCallbackValue::NodeEventCallbackValue(
		FMaxLiveLink* maxLink,
		Interface* ip
	)
		: maxLink_( maxLink )
		, interface_( ip )
	{
		this->nodeEventKey_	=
			GetISceneEventManager()->RegisterCallback(
				this,
				FALSE, 50,
				TRUE
			);
		this->interface_->RegisterViewportDisplayCallback( TRUE, this );
	}

	NodeEventCallbackValue::~NodeEventCallbackValue()
	{
		this->interface_->UnRegisterViewportDisplayCallback( TRUE, this );
		GetISceneEventManager()->UnRegisterCallback( this->nodeEventKey_ );

	}

	void	NodeEventCallbackValue::ModelStructured( NodeKeyTab& nodes )
	{
	}
	void	NodeEventCallbackValue::GeometryChanged( NodeKeyTab& nodes )
	{
		this->maxLink_->ApplyCurrentFrameSubjects();
	}
	void	NodeEventCallbackValue::MappingChanged( NodeKeyTab& nodes )
	{
		TArray<INode*> nodeList;
		for( int iNode = 0; iNode < nodes.Count(); ++iNode )
		{
			INode* node = NodeEventNamespace::GetNodeByKey( nodes[ iNode ] );
			if( node )
			{
				if( node->Selected() )
				{
					nodeList.Add( node );
				}
			}
		}
		if(
			nodeList.Num() > 0 &&
			this->maxLink_->UseAutomaticSyncMesh()
			)
		{
			if( this->maxLink_->IsContainNodesFromAnySubjects( nodeList ) )
			{
				this->maxLink_->SettingAllSkinSubjectUpdateGeometry( true );
			}
			this->maxLink_->ApplyCurrentFrameSubjects();
		}
	}

	void	NodeEventCallbackValue::NameChanged( NodeKeyTab& nodes )
	{
		this->maxLink_->UpdateAllSubjectNodeListReference();
	}
	void	NodeEventCallbackValue::Added( NodeKeyTab& nodes )
	{
		this->maxLink_->UpdateAllSubjectNodeListReference();
	}
	void	NodeEventCallbackValue::Deleted( NodeKeyTab& nodes )
	{
		this->maxLink_->UpdateAllSubjectNodeListReference();
	}
	void	NodeEventCallbackValue::LinkChanged( NodeKeyTab& nodes )
	{
		this->maxLink_->UpdateAllSubjectNodeListReference();
	}
	void	NodeEventCallbackValue::LayerChanged( NodeKeyTab& nodes )
	{
		this->maxLink_->UpdateAllSubjectNodeListReference();
	}
	void	NodeEventCallbackValue::GroupChanged( NodeKeyTab& nodes )
	{
		this->maxLink_->UpdateAllSubjectNodeListReference();
	}
	void	NodeEventCallbackValue::HierarchyOtherEvent( NodeKeyTab& nodes )
	{
		this->maxLink_->UpdateAllSubjectNodeListReference();
	}

	void	NodeEventCallbackValue::Display( TimeValue t, ViewExp *vpt, int flags )
	{
		if( vpt->IsActive() )
		{
			this->maxLink_->OnDisplay( t, vpt, flags );
		}
	}


	void	NodeEventCallbackValue::GetViewportRect( TimeValue t, ViewExp* vpt, Rect* rect )
	{
	}

	BOOL	NodeEventCallbackValue::Foreground()
	{
		return TRUE;
	}

/*------- ↑ ノード更新コールバック ↑ ------- }}} */






/*! @class	FMaxOutputDevice
	@brief	ログ出力
*/
class FMaxOutputDevice : public FOutputDevice
{
public:
	FMaxOutputDevice() : bAllowLogVerbosity( false ) {}

	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category ) override
	{
		if( ( bAllowLogVerbosity && Verbosity <= ELogVerbosity::Log ) || ( Verbosity <= ELogVerbosity::Display ) )
		{
			_tprintf( V );
		}
	}

private:

	bool bAllowLogVerbosity;

};





/*------- ↓ プラグイン挙動と汎用処理 ↓ ------- {{{ */



	FMaxLiveLink::FMaxLiveLink(
		HINSTANCE instanceHandle
	)
		: instanceHandle_( instanceHandle )
		, controlPageHandle_( nullptr )
		, nodeEvent_( nullptr )
		, interface_( nullptr )
		// UI操作
		, windowHandle_( nullptr )
		, editViewCameraSubject_( nullptr )

		, buttonAddSubject_( nullptr )
		, buttonRemoveSubject_( nullptr )

		, textSubjectName_( nullptr )
		, listSubject_( nullptr )

		, useForceFrontX_Cache_( true )
		, useAutomaticSyncMesh_Cache_( false )

		, buttonSendMesh_( nullptr )

		, numUnitScale_( nullptr )
			, unitScale_Cache_( 1.0f )
		, numVertexUVs_( nullptr )
			, vertexUVs_Cache_( 1 )
		, numVertexColors_( nullptr )
			, vertexColors_Cache_( 1 )

		, bindPoseFrameType_( BindPoseFrameType_FirstFrame )
		, numBindPoseFrame_( nullptr )
			, numBindPoseFrame_Cache_( 0 )
		
		// 更新受け取りと送信
		, lastViewID_( -1 )
	{
	}

	FMaxLiveLink::~FMaxLiveLink()
	{
	}




	void	FMaxLiveLink::Init( HWND hPanel )
	{
	}

	void	FMaxLiveLink::Destroy( HWND hWnd )
	{
		if( this->liveLinkProvider_.IsValid() )
		{
			if( this->connectionStatusChangedHandle_.IsValid() )
			{
				this->liveLinkProvider_->UnregisterConnStatusChangedHandle( this->connectionStatusChangedHandle_ );
				this->connectionStatusChangedHandle_.Reset();
			}
			this->liveLinkProvider_ = nullptr;
		}
	}








	/*------- ↓ UI操作 ↓ ------- {{{ */

		void OnConnectionStatusChanged()
		{
		}

		/*! @brief		カスタムコントロールダイアログのWindowsプロセス
		*/
		INT_PTR CALLBACK CustomControlDialogProcess(
			HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam 
		) 
		{
			FMaxLiveLink *u = DLGetWindowLongPtr<FMaxLiveLink *>( hWnd );
			if( !u && message != WM_INITDIALOG ) 
			{
				return FALSE;
			}

			switch( message )
			{
			case WM_INITDIALOG: 
				{
					u = ( FMaxLiveLink * )lParam;
					DLSetWindowLongPtr( hWnd, u );
					u->InitializeDialog( hWnd );
					return TRUE;
				}
			default:
				return u->ReceiveCustomControlDialogProcess(
					message, wParam, lParam
				);
			}
			return TRUE;
		}

		/*! @brief		Utilityプラグインとしての使用を開始
		*/
		void	FMaxLiveLink::BeginEditParams( Interface *ip, IUtil *iu )
		{
			if( !this->nodeEvent_ )
			{
				Sleep( 0 );
				GEngineLoop.PreInit( TEXT( "MaxLiveLinkPlugin -Messaging" ) );
				ProcessNewlyLoadedUObjects();

				FModuleManager::Get().StartProcessingNewlyLoadedObjects();
				FModuleManager::Get().LoadModule( TEXT( "UdpMessaging" ) );

				this->nodeEvent_ = new NodeEventCallbackValue( this, ip );

				GLog->TearDown(); //clean up existing output devices
				GLog->AddOutputDevice( new FMaxOutputDevice() ); //Add Maya output device

				UE_LOG( LogMaxPlugin, Display, TEXT( "MaxLiveLinkPlugin initialized" ) );

				this->liveLinkProvider_ = ILiveLinkProvider::CreateLiveLinkProvider( TEXT( "Max Live Link" ) );
				this->connectionStatusChangedHandle_ =
					this->liveLinkProvider_->RegisterConnStatusChangedHandle(
						FLiveLinkProviderConnectionStatusChanged::FDelegate::CreateStatic( &OnConnectionStatusChanged )
					);
				this->interface_		= ip;
				this->editViewCameraSubject_ = 
					new FMaxLiveLinkCameraStreamedSubject(
						this,
						TEXT( "EditorActiveCamera" )
					);
		
			}

			auto resource = MAKEINTRESOURCE( IDD_LIVELINK_PAGE );
			this->controlPageHandle_ = 
				ip->AddRollupPage(
					this->instanceHandle_,
					resource,
					CustomControlDialogProcess,
					_T( "LiveLink Controls" ),
					( LPARAM )this 
				);
		}

		/*! @brief		Utilityプラグインとしての使用を終了
		*/
		void	FMaxLiveLink::EndEditParams( Interface *ip, IUtil *iu )
		{
			ip->DeleteRollupPage( this->controlPageHandle_ );
			this->controlPageHandle_ = nullptr;
		}


		/*! @brief		ダイアログのWindowsプロセス
		*/
		INT_PTR		FMaxLiveLink::ReceiveCustomControlDialogProcess(
			UINT message, WPARAM wParam, LPARAM lParam
		)
		{
			switch( message )
			{
			case WM_DESTROY:
				{
					this->FinalizeDialog();
				}break;
			case WM_COMMAND:
				{
					this->OnReceiveDialogCallbackCommand( wParam );
				}break;
			case WM_CUSTEDIT_ENTER:
				{
					this->OnReceiveEnterEditCallbackCommand( wParam );
				}break;
			default:
				return FALSE;
			}
			return TRUE;
		}



		/*! @brief		ダイアログ内容の初期化
		*/
		void	FMaxLiveLink::InitializeDialog( HWND hWnd )
		{
			this->windowHandle_		= hWnd;

			this->buttonAddSubject_	= GetICustButton( GetDlgItem( this->windowHandle_, IDC_BUTTON_ADD_SUBJECT ) );
			if( this->buttonAddSubject_ )
			{
				this->buttonAddSubject_->SetType( CBT_PUSH );
				this->buttonAddSubject_->SetRightClickNotify( TRUE );
				this->buttonAddSubject_->SetButtonDownNotify( TRUE );
				this->buttonAddSubject_->SetText( TEXT( "Add" ) );
			}

			this->buttonRemoveSubject_	= GetICustButton( GetDlgItem( this->windowHandle_, IDC_BUTTON_REMOVE_SUBJECT ) );
			if( this->buttonRemoveSubject_ )
			{
				this->buttonRemoveSubject_->SetType( CBT_PUSH );
				this->buttonRemoveSubject_->SetRightClickNotify( TRUE );
				this->buttonRemoveSubject_->SetButtonDownNotify( TRUE );
				this->buttonRemoveSubject_->SetText( TEXT( "Remove" ) );
			}

			this->textSubjectName_	= GetICustEdit( GetDlgItem( this->windowHandle_, IDC_SUBJECTNAME ) );
			this->listSubject_		= GetDlgItem( this->windowHandle_, IDC_SUBJECT_LIST );
	
			this->buttonSendMesh_	= GetICustButton( GetDlgItem( this->windowHandle_, IDC_BUTTON_SendMeshForce ) );
			if( this->buttonSendMesh_ )
			{
				this->buttonSendMesh_->SetType( CBT_PUSH );
				this->buttonSendMesh_->SetRightClickNotify( TRUE );
				this->buttonSendMesh_->SetButtonDownNotify( TRUE );
				this->buttonSendMesh_->SetText( TEXT( "Send Mesh" ) );
			}

			CheckDlgButton(
				this->windowHandle_, IDC_CHECK_ForceFrontX,
				this->useForceFrontX_Cache_ ? BST_CHECKED: BST_UNCHECKED
			);

			CheckDlgButton(
				this->windowHandle_, IDC_CHECK_AutomaticSyncMesh,
				this->useAutomaticSyncMesh_Cache_ ? BST_CHECKED: BST_UNCHECKED
			);

			this->numUnitScale_	= GetISpinner( GetDlgItem( this->windowHandle_, IDC_NUMERIC_UnitScale ) );
			if( this->numUnitScale_ )
			{
				this->numUnitScale_->SetScale( 0.001f );
				this->numUnitScale_->SetLimits( 0.0f, 10000.0f );
				this->numUnitScale_->SetResetValue( 1.0f );
				this->numUnitScale_->SetValue( this->unitScale_Cache_, 0 );
				this->numUnitScale_->LinkToEdit( GetDlgItem( this->windowHandle_, IDC_NUMERIC_UnitScale_Text ), EDITTYPE_FLOAT );
			}


			this->numVertexUVs_	= GetISpinner( GetDlgItem( this->windowHandle_, IDC_NUMERIC_NumUV ) );
			if( this->numVertexUVs_ )
			{
				this->numVertexUVs_->SetScale( 1 );
				this->numVertexUVs_->SetLimits( 0, 4 );
				this->numVertexUVs_->SetResetValue( 1 );
				this->numVertexUVs_->SetValue( this->vertexUVs_Cache_, 0 );
				this->numVertexUVs_->LinkToEdit( GetDlgItem( this->windowHandle_, IDC_NUMERIC_NumUV_Text ), EDITTYPE_INT );
			}

			this->numVertexColors_	= GetISpinner( GetDlgItem( this->windowHandle_, IDC_NUMERIC_NumVertexColors ) );
			if( this->numVertexColors_ )
			{
				this->numVertexColors_->SetScale( 1 );
				this->numVertexColors_->SetLimits( 0, 2 );
				this->numVertexColors_->SetResetValue( 1 );
				this->numVertexColors_->SetValue( this->vertexColors_Cache_, 0 );
				this->numVertexColors_->LinkToEdit( GetDlgItem( this->windowHandle_, IDC_NUMERIC_NumVertexColors_Text ), EDITTYPE_INT );
			}

			

			{
				auto selectedButton = IDC_RADIO_BindPose_FirstFrame;
				switch( this->bindPoseFrameType_ )
				{
				default:
				case BindPoseFrameType_FirstFrame:
					{
						selectedButton = IDC_RADIO_BindPose_FirstFrame;
					}break;
				case BindPoseFrameType_SelectFrame:
					{
						selectedButton = IDC_RADIO_BindPose_SelectFrame;
					}break;
				}
				CheckRadioButton(
					this->windowHandle_,
					IDC_RADIO_BindPose_FirstFrame,
					IDC_RADIO_BindPose_SelectFrame,
					selectedButton
				);

				this->numBindPoseFrame_	= GetISpinner( GetDlgItem( this->windowHandle_, IDC_NUMERIC_BindPoseFrame ) );
				if( this->numBindPoseFrame_ )
				{
					this->numBindPoseFrame_->SetScale( 1 );
					this->numBindPoseFrame_->SetResetValue( 1 );
					this->numBindPoseFrame_->SetValue( this->numBindPoseFrame_Cache_, 0 );
					this->numBindPoseFrame_->LinkToEdit( GetDlgItem( this->windowHandle_, IDC_NUMERIC_BindPoseFrame_Text ), EDITTYPE_INT );
				}
			}

			{
				SendMessageW( this->listSubject_, LB_RESETCONTENT, 0, 0 );
				for( const auto& skinPair : this->skinNodeSubjectList_ )
				{
					auto skinName = *skinPair.Key;
					SendMessageW( this->listSubject_, LB_ADDSTRING, 0, ( LPARAM )skinName );
				}
			}
		}

		/*! @brief		ダイアログ内容の解放
		*/
		void	FMaxLiveLink::FinalizeDialog()
		{
			this->useForceFrontX_Cache_				= this->UseForceFrontAxisX();
			this->useAutomaticSyncMesh_Cache_		= this->UseAutomaticSyncMesh();


			{
				this->bindPoseFrameType_	= BindPoseFrameType_FirstFrame;
				if( IsDlgButtonChecked( this->windowHandle_, IDC_RADIO_BindPose_FirstFrame ) )
				{
					this->bindPoseFrameType_	= BindPoseFrameType_FirstFrame;
				}
				else if( IsDlgButtonChecked( this->windowHandle_, IDC_RADIO_BindPose_SelectFrame ) )
				{
					this->bindPoseFrameType_	= BindPoseFrameType_SelectFrame;
				}
			}

			if( this->numBindPoseFrame_ )
			{
				this->numBindPoseFrame_Cache_ = this->numBindPoseFrame_->GetIVal();
				ReleaseISpinner( this->numBindPoseFrame_ );
				this->numBindPoseFrame_ = nullptr;
			}

			if( this->numVertexColors_ )
			{
				this->vertexColors_Cache_ = this->numVertexColors_->GetIVal();
				ReleaseISpinner( this->numVertexColors_ );
				this->numVertexColors_ = nullptr;
			}
			if( this->numVertexUVs_ )
			{
				this->vertexUVs_Cache_ = this->numVertexUVs_->GetIVal();
				ReleaseISpinner( this->numVertexUVs_ );
				this->numVertexUVs_ = nullptr;
			}

			if( this->numUnitScale_ )
			{
				this->unitScale_Cache_ = this->numUnitScale_->GetFVal();
				ReleaseISpinner( this->numUnitScale_ );
				this->numUnitScale_ = nullptr;
			}

			if( this->buttonSendMesh_ )
			{
				ReleaseICustButton( this->buttonSendMesh_ );
				this->buttonSendMesh_ = nullptr;
			}

			if( this->buttonSendMesh_ )
			{
				ReleaseICustButton( this->buttonSendMesh_ );
				this->buttonSendMesh_ = nullptr;
			}

			{
				this->listSubject_ = nullptr;
			}
			if( this->textSubjectName_ )
			{
				ReleaseICustEdit( this->textSubjectName_ );
				this->textSubjectName_ = nullptr;
			}

			if( this->buttonRemoveSubject_ )
			{
				ReleaseICustButton( this->buttonRemoveSubject_ );
				this->buttonRemoveSubject_ = nullptr;
			}

			if( this->buttonAddSubject_ )
			{
				ReleaseICustButton( this->buttonAddSubject_ );
				this->buttonAddSubject_ = nullptr;
			}
			
			this->windowHandle_ = nullptr;
		}




		/*! @brief		+Xを正面とさせるか
		*/
		bool	FMaxLiveLink::UseForceFrontAxisX() const
		{
			if( !this->windowHandle_ )
			{
				return this->useForceFrontX_Cache_;
			}
			return IsDlgButtonChecked( this->windowHandle_, IDC_CHECK_ForceFrontX ) == TRUE;
		}

		/*! @brief		自動でのメッシュ同期を使用するか
		*/
		bool	FMaxLiveLink::UseAutomaticSyncMesh() const
		{
			if( !this->windowHandle_ )
			{
				return this->useAutomaticSyncMesh_Cache_;
			}
			return IsDlgButtonChecked( this->windowHandle_, IDC_CHECK_AutomaticSyncMesh ) == TRUE;
		}



		/*! @brief		単位のスケールを取得
		*/
		float	FMaxLiveLink::GetUnitScale() const
		{
			if( !this->numUnitScale_ )
			{
				return this->unitScale_Cache_;
			}
			return this->numUnitScale_->GetFVal();
		}

		/*! @brief		使用するUVの数を取得
		*/
		int		FMaxLiveLink::GetNumUseUVs() const
		{
			if( !this->numVertexUVs_ )
			{
				return this->vertexUVs_Cache_;
			}
			return this->numVertexUVs_->GetIVal();
		}

		/*! @brief		使用するカラーの数を取得
		*/
		int		FMaxLiveLink::GetNumUseColors() const
		{
			if( !this->numVertexColors_ )
			{
				return this->vertexColors_Cache_;
			}
			return this->numVertexColors_->GetIVal();
		}



		/*! @brief		ダイアログコールバック命令の受け取り
		*/
		void	FMaxLiveLink::OnReceiveDialogCallbackCommand( WPARAM wParam )
		{
			switch( LOWORD( wParam ) )
			{
			case IDC_BUTTON_ADD_SUBJECT:
				{
					if( HIWORD( wParam ) == BN_BUTTONUP )
					{
						this->AddEditedSubject();
					}
				}break;

			case IDC_BUTTON_REMOVE_SUBJECT:
				{
					if( HIWORD( wParam ) == BN_BUTTONUP )
					{
						this->RemoveSelectedSubject();
					}
				}break;


			case IDC_BUTTON_SendMeshForce:
				{
					if( HIWORD( wParam ) == BN_BUTTONUP )
					{
						this->ApplyCurrentFrameSubjects( true );
					}
				}break;

			default:
				break;
			}
		}

		/*! @brief	テキストエディットのコールバック命令の受け取り	
		*/
		void	FMaxLiveLink::OnReceiveEnterEditCallbackCommand( WPARAM wParam )
		{
			switch( wParam )
			{
			case IDC_SUBJECTNAME:
				{
					this->AddEditedSubject();
				}break;
			default:
				break;
			}
		}

	/*------- ↑ UI操作 ↑ ------- }}} */



	/*------- ↓ Subject管理 ↓ ------- {{{ */


		/*! @brief		Subject用ノードの収集を再帰的に行う
		*/
		void	FMaxLiveLink::CollectSubjectNodes_Recursive(
			NodeArray& outArray, INode* currentNode,
			uint32 collectOption
		)
		{
			if( CollectOptionBits_AllNode & collectOption )
			{
				outArray.AddUnique( currentNode );
			}
			else
			{
				auto nodeName = currentNode->GetName();

				auto cID	= currentNode->ClassID();
				auto scID	= currentNode->SuperClassID();

				auto derivedObject	= reinterpret_cast<IDerivedObject*>( currentNode->GetObjectRef() );
				if(
					derivedObject &&
					( !( collectOption & CollectOptionBits_OnlySelected ) ) ||
					( ( collectOption & CollectOptionBits_OnlySelected ) && currentNode->Selected() )
					)
				{
					bool isTargetType = false;
					auto superClassID	= derivedObject->SuperClassID();
					auto dcID			= derivedObject->ClassID();

					if( CollectOptionBits_SkinedMesh & collectOption )
					{
						int numModifier = derivedObject->NumModifiers();
						for( int iModifier = 0; iModifier < numModifier; ++iModifier )
						{
							auto modifier = derivedObject->GetModifier( iModifier );
							if( modifier )
							{

								auto skin = reinterpret_cast<ISkin*>( modifier->GetInterface( I_SKIN ) );
								if( skin )
								{
									isTargetType = true;
								}
							}
						}
					}

					if( CollectOptionBits_MorphMesh & collectOption )
					{
						int numModifier = derivedObject->NumModifiers();
						for( int iModifier = 0; iModifier < numModifier; ++iModifier )
						{
							auto modifier = derivedObject->GetModifier( iModifier );
							if( modifier )
							{
								if(
									modifier->SuperClassID() == OSM_CLASS_ID &&
									modifier->ClassID() == MORPHER_CLASS_ID
									)
								{

									isTargetType = true;
								}
							}
						}
					}

					if( CollectOptionBits_Mesh & collectOption )
					{
						if(
							superClassID == GEOMOBJECT_CLASS_ID
							|| superClassID == TRIOBJ_CLASS_ID
							|| superClassID == POLYOBJ_CLASS_ID
							)
						{
							isTargetType = true;
						}
					}
					if( CollectOptionBits_Bone & collectOption )
					{
						if( superClassID == BONE_CLASS_ID )
						{
							isTargetType = true;
						}
					}
					if( CollectOptionBits_Camera & collectOption )
					{
						if( superClassID == CAMERA_CLASS_ID )
						{
							isTargetType = true;
						}
					}

					if( CollectOptionBits_Helper & collectOption )
					{
						if( superClassID == HELPER_CLASS_ID )
						{
							isTargetType = true;
						}
					}

					if( CollectOptionBits_Dummy & collectOption )
					{
						if( superClassID == DUMMY_CLASS_ID )
						{
							isTargetType = true;
						}
					}


					if( isTargetType )
					{
						outArray.AddUnique( currentNode );
					}
				}
			}

			int numChildren = currentNode->NumberOfChildren();
			for( int iChild = 0; iChild < numChildren; ++iChild )
			{
				INode* child = currentNode->GetChildNode( iChild );
				if( child )
				{
					this->CollectSubjectNodes_Recursive(
						outArray, child,
						collectOption
					);
				}
			}
		}

		/*! @brief		Subject用ノードの収集
		*/
		NodeArray	FMaxLiveLink::CollectSubjectNodes(
			uint32 collectOption
		)
		{
			NodeArray ret;
			this->CollectSubjectNodes(
				ret,
				collectOption
			);

			return ret;
		}

		/*! @brief		Subject用ノードの収集
		*/
		void	FMaxLiveLink::CollectSubjectNodes(
			NodeArray& outArray,
			uint32 collectOption
		)
		{
			outArray.Empty();

			Interface14* interFace = GetCOREInterface14();
			if( !interFace )
			{
				return;
			}

			INode* rootNode = interFace->GetRootNode();

			this->CollectSubjectNodes_Recursive(
				outArray, rootNode,
				collectOption
			);
		}


		/*! @brief		編集されるSubjectの追加
			@details	追加されるSubjectはUIの状態にて設定される
		*/
		void	FMaxLiveLink::AddEditedSubject()
		{
			TCHAR subjectName[ 256 ];
			int sizeOfText = this->textSubjectName_->GetTextLength();
			if( sizeOfText > 0 )
			{
				this->textSubjectName_->GetText( subjectName, 256 );

				auto oldSubject = this->skinNodeSubjectList_.Find( subjectName );
				if( oldSubject )
				{
					this->skinNodeSubjectList_.Remove( subjectName );
					delete ( *oldSubject );
				}

				NodeArray skinNodeList;
				{
					NodeArray selectedNodeList = 
						this->CollectSubjectNodes(
							CollectOptionBits_OnlySelected |
							CollectOptionBits_SkinedMesh |
							CollectOptionBits_MorphMesh |
							CollectOptionBits_Mesh |
							CollectOptionBits_Helper |
							CollectOptionBits_Dummy
						);
					if( selectedNodeList.Num() == 0 )
					{
						skinNodeList =
							this->CollectSubjectNodes(
								CollectOptionBits_SkinedMesh |
								CollectOptionBits_MorphMesh |
								CollectOptionBits_Mesh |
								CollectOptionBits_Helper |
								CollectOptionBits_Dummy
							);
					}
					else
					{
						skinNodeList = selectedNodeList;
					}
				}

		
				{
					auto stream =
						new FMaxLiveLinkSkinMeshesStreamedSubject(
							this,
							skinNodeList, subjectName
						);
					this->skinNodeSubjectList_.Add( subjectName, stream );
				}

				{
					SendMessageW( this->listSubject_, LB_RESETCONTENT, 0, 0 );
					for( const auto& skinPair : this->skinNodeSubjectList_ )
					{
						auto skinName = *skinPair.Key;
						SendMessageW( this->listSubject_, LB_ADDSTRING, 0, ( LPARAM )skinName );
					}
				}
				this->lastUpdateTime_ = TIME_PosInfinity;
			}
		}

		/*! @brief		選択されているSubjectの削除
		*/
		void	FMaxLiveLink::RemoveSelectedSubject()
		{
			int selectedIndexList[ 256 ];
			auto numSelected = SendMessageW( this->listSubject_, LB_GETSELCOUNT, 256, ( LPARAM )selectedIndexList );
			SendMessageW( this->listSubject_, LB_GETSELITEMS, 256, ( LPARAM )selectedIndexList );


			for( int iSelect = 0; iSelect < numSelected; ++iSelect )
			{
				TCHAR subjectName[ 256 ];

				auto textLen	= SendMessageW( this->listSubject_, LB_GETTEXTLEN, selectedIndexList[ iSelect ], 0 );
				auto getResult	= SendMessageW( this->listSubject_, LB_GETTEXT, selectedIndexList[ iSelect ], ( LPARAM )subjectName );

				auto oldSubject = this->skinNodeSubjectList_.Find( subjectName );
				if( oldSubject )
				{
					this->skinNodeSubjectList_.Remove( subjectName );
					delete ( *oldSubject );
				}
			}

			{
				SendMessageW( this->listSubject_, LB_RESETCONTENT, 0, 0 );
				for( const auto& skinPair : this->skinNodeSubjectList_ )
				{
					auto skinName = *skinPair.Key;
					SendMessageW( this->listSubject_, LB_ADDSTRING, 0, ( LPARAM )skinName );
				}
			}
		}


		/*! @brief		すべてのSkinのSubjectを削除
		*/
		void	FMaxLiveLink::DeleteAllSkinNodeSubject()
		{
			SendMessageW( this->listSubject_, LB_RESETCONTENT, 0, 0 );

			for( const auto& subjectpair : this->skinNodeSubjectList_ )
			{
				auto subjectObject = subjectpair.Value;
				delete ( subjectObject );
			}
			this->skinNodeSubjectList_.Empty();
		}



		/*! @brief		更新されたジオメトリーのノードを受け取り
		*/
		void	FMaxLiveLink::ReceiveChangeGeometoryNodes( const TArray<INode*>& nodes )
		{
			for( const auto& subjectpair : this->skinNodeSubjectList_ )
			{
				subjectpair.Value->ReceiveUpdateGeometryOfNodes( nodes );
			}
		}


		/*! @brief		ノードがSubjectのどれかによって保持されているか
		*/
		bool	FMaxLiveLink::IsContainNodesFromAnySubjects( const TArray<INode*>& nodes ) const
		{
			for( const auto& subjectpair : this->skinNodeSubjectList_ )
			{
				if( subjectpair.Value->IsContainNodes( nodes ) )
				{
					return true;
				}
			}
			return false;
		}


		/*! @brief		すべてスキンSubjectのメッシュ同期の最終更新時のキャッシュをクリア
		*/
		void	FMaxLiveLink::ClearAllSkinSubjectSyncDataCache()
		{
			for( const auto& subjectpair : this->skinNodeSubjectList_ )
			{
				subjectpair.Value->ClearSyncDataCache();
			}
		}
		/*! @brief		すべてスキンSubjectのジオメトリーの更新状態を設定
		*/
		void	FMaxLiveLink::SettingAllSkinSubjectUpdateGeometry( bool doUpdate )
		{
			for( const auto& subjectpair : this->skinNodeSubjectList_ )
			{
				subjectpair.Value->SettingUpdateGeometry( doUpdate );
			}
		}



		/*! @brief		すべてのSubjectのノードリストの内容が存在しているかを確認して参照状態を更新
		*/
		void	FMaxLiveLink::UpdateAllSubjectNodeListReference()
		{
			TArray< FString > deleteSubjectList;
			for( auto stream : this->skinNodeSubjectList_ )
			{
				stream.Value->UpdateNodeListReference();
				if( !stream.Value->HasAnyNodeReferences() )
				{
					deleteSubjectList.Add( stream.Key );
				}
			}
			for( const auto& subjectKey : deleteSubjectList )
			{
				auto subject = this->skinNodeSubjectList_.Find( subjectKey );
				if( subject )
				{
					this->skinNodeSubjectList_.Remove( subjectKey );
					delete ( *subject );
				}
			}
		}

	/*------- ↑ Subject管理 ↑ ------- }}} */


	/*------- ↓ 更新受け取りと送信 ↓ ------- {{{ */


		/*! @brief		バインドポーズ用の時間を取得
		*/
		TimeValue	FMaxLiveLink::GetBindPoseTime() const
		{
			auto frameType	= this->bindPoseFrameType_;
			if( IsDlgButtonChecked( this->windowHandle_, IDC_RADIO_BindPose_FirstFrame ) )
			{
				frameType	= BindPoseFrameType_FirstFrame;
			}
			else if( IsDlgButtonChecked( this->windowHandle_, IDC_RADIO_BindPose_SelectFrame ) )
			{
				frameType	= BindPoseFrameType_SelectFrame;
			}

			switch( frameType )
			{
			default:
			case BindPoseFrameType_FirstFrame:
				{
					auto range = this->interface_->GetAnimRange();
					return range.Start();
				}
			case BindPoseFrameType_SelectFrame:
				{
					if( !this->numBindPoseFrame_ )
					{
						return this->numBindPoseFrame_Cache_;
					}
					return this->numBindPoseFrame_->GetIVal();
				}
			}
			
		}

		/*! @brief		描画の受け取り
		*/
		void	FMaxLiveLink::OnDisplay( TimeValue t, ViewExp* vpt, int flags )
		{
			double	frameSeconds	= TicksToSec( t );
			int		frame			= t / GetTicksPerFrame();

			if( 
				this->lastUpdateTime_ != t ||
				this->lastViewID_ != vpt->GetViewID()
			)
			{
				this->lastUpdateTime_	= t;
				this->lastViewID_		= vpt->GetViewID();
		

				{
					this->UpdateAllSubjectNodeListReference();

					if( !this->UseAutomaticSyncMesh() )
					{
						for( auto stream : this->skinNodeSubjectList_ )
						{
							stream.Value->SettingUpdateGeometry( false );
						}
					}

					for( auto stream : this->skinNodeSubjectList_ )
					{
						stream.Value->OnStream( frameSeconds, frame );
					}
				}
			}
			auto vp13 = reinterpret_cast<ViewExp13*>( vpt->Execute( ViewExp::kEXECUTE_GET_VIEWEXP_13 ) );
			if( vp13 )
			{
				{
					for( auto stream : this->cameraSubjectList_ )
					{
						stream->OnStreamCamera( frameSeconds, frame, vp13, flags );
					}
				}

				if( this->editViewCameraSubject_ )
				{
					this->editViewCameraSubject_->OnStreamCamera( frameSeconds, frame, vp13, flags );
				}
			}
		}

		/*! @brief		現在のフレームのSubjectの状態を適応
		*/
		void	FMaxLiveLink::ApplyCurrentFrameSubjects( bool forceUpdateMesh )
		{
			this->lastUpdateTime_ = this->interface_->GetTime();


			double	frameSeconds	= TicksToSec( this->lastUpdateTime_ );
			int		frame			= this->lastUpdateTime_ / GetTicksPerFrame();

			{
				for( auto stream : this->skinNodeSubjectList_ )
				{
					if( forceUpdateMesh )
					{
						stream.Value->ClearSyncDataCache();
						stream.Value->SettingUpdateGeometry( true );
					}
					stream.Value->OnStream( frameSeconds, frame );
				}
			}
		}

	/*------- ↑ 更新受け取りと送信 ↑ ------- }}} */


	/*------- ↓ 汎用行列取得 ↓ ------- {{{ */


		const Matrix3	FMaxLiveLink::FrontXMatrix(
			Point3( 0.0f, 1.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 1.0f ),
			Point3( 1.0f, 0.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 0.0f )
		);

		const Matrix3&	FMaxLiveLink::GetFrontXMatrix()
		{
			return FMaxLiveLink::FrontXMatrix;
		}



		const Matrix3	FMaxLiveLink::InvertFrontZMatrix(
			Point3( 1.0f, 0.0f, 0.0f ),
			Point3( 0.0f, 0.0f, -1.0f ),
			Point3( 0.0f, 1.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 0.0f )
		);

		const Matrix3&	FMaxLiveLink::GetInvertFrontZMatrix()
		{
			return InvertFrontZMatrix;
		}




		const Matrix3	FMaxLiveLink::FrontXRootMatrix(
			Point3( 0.0f, 0.0f, 1.0f ),
			Point3( 1.0f, 0.0f, 0.0f ),
			Point3( 0.0f, 1.0f, 0.0f ),
			Point3( 0.0f, 0.0f, 0.0f )
		);

		const Matrix3&	FMaxLiveLink::GetFrontXRootMatrix()
		{
			return FMaxLiveLink::FrontXRootMatrix;
		}

	/*------- ↑ 汎用行列取得 ↑ ------- }}} */


/*------- ↑ プラグイン挙動と汎用処理 ↑ ------- }}} */
