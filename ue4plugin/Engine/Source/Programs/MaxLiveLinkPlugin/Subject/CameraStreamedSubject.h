
/*! @file		CameraStreamedSubject.h
	@brief		カメラ同期用のSubjectを管理する構成
*/

#pragma once

#if !defined( __CameraStreamedSubject__ )
	#define __CameraStreamedSubject__


	#include	"BaseStreamedSubject.h"

	/*! @class		FMaxLiveLinkCameraStreamedSubject
		@brief		カメラ同期用のSubjectを管理する構成
	*/
	class FMaxLiveLinkCameraStreamedSubject
		: public FMaxLiveLinkBaseStreamedSubject
	{
	public:
		FMaxLiveLinkCameraStreamedSubject(
			FMaxLiveLink* livelinkReference,
			INode* node, const FName& subjectName
		);

		FMaxLiveLinkCameraStreamedSubject(
			FMaxLiveLink* livelinkReference,
			const FName& subjectName
		);

		virtual ~FMaxLiveLinkCameraStreamedSubject();

	public:
		static const Matrix3	ToUE4Matrix;
		static const Matrix3	ToFrontAxisXMatrix;

	public:

		//----------------------------------------------------------
		//! Subjectとしてのデータを構築
		virtual void BuildSubjectData();
		//! UE4へのデータ送信
		virtual void OnStream( double streamTime, int32 frameNumber );

		//! カメラとしてのUE4へのデータ送信
		void	OnStreamCamera( double streamTime, int frame, ViewExp13* vpt, int flags );
		//----------------------------------------------------------
	};


#endif // __CameraStreamedSubject__

