
/*! @file		CameraStreamedSubject.h
	@brief		�J���������p��Subject���Ǘ�����\��
*/

#pragma once

#if !defined( __CameraStreamedSubject__ )
	#define __CameraStreamedSubject__


	#include	"BaseStreamedSubject.h"

	/*! @class		FMaxLiveLinkCameraStreamedSubject
		@brief		�J���������p��Subject���Ǘ�����\��
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
		//! Subject�Ƃ��Ẵf�[�^���\�z
		virtual void BuildSubjectData();
		//! UE4�ւ̃f�[�^���M
		virtual void OnStream( double streamTime, int32 frameNumber );

		//! �J�����Ƃ��Ă�UE4�ւ̃f�[�^���M
		void	OnStreamCamera( double streamTime, int frame, ViewExp13* vpt, int flags );
		//----------------------------------------------------------
	};


#endif // __CameraStreamedSubject__

