#include "pch.h"
#include "RubikCube.h"
#include "BasicTimer.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace concurrency;

RubikCube::RubikCube() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

void RubikCube::Initialize(CoreApplicationView^ applicationView)
{
	applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &RubikCube::OnActivated);

	CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &RubikCube::OnSuspending);

	CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &RubikCube::OnResuming);

	m_renderer = ref new CubeRenderer();
}

void RubikCube::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &RubikCube::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &RubikCube::OnVisibilityChanged);

	window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &RubikCube::OnWindowClosed);

	window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &RubikCube::OnPointerPressed);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &RubikCube::OnPointerMoved);

	window->PointerReleased +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &RubikCube::OnPointerReleased);

	window->PointerWheelChanged +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &RubikCube::OnPointerWheelChanged);

	// Quit when user press Esc key
	window->KeyUp +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &RubikCube::OnKeyUp);

	m_renderer->Initialize(CoreWindow::GetForCurrentThread());
}

void RubikCube::Load(Platform::String^ entryPoint)
{
}

void RubikCube::Run()
{
	BasicTimer^ timer = ref new BasicTimer();

	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			timer->Update();
			m_renderer->Update(timer->Total, timer->Delta);

			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			m_renderer->Render();
			m_renderer->Present(); // This call is synchronized to the display frame rate.
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void RubikCube::Uninitialize()
{
}

void RubikCube::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_renderer->UpdateForWindowSizeChange();
}

void RubikCube::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void RubikCube::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

void RubikCube::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	m_renderer->OnPointerPressed(sender, args);
}

void RubikCube::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
	m_renderer->OnPointerMoved(sender, args);
}

void RubikCube::OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ args)
{
	m_renderer->OnPointerReleased(sender, args);
}

void RubikCube::OnPointerWheelChanged(CoreWindow^ sender, PointerEventArgs^ args)
{
	m_renderer->OnMouseWheelRotate(sender, args);
}

void RubikCube::OnKeyUp(CoreWindow^ sender, KeyEventArgs^ args)
{
	// Quit when user press Esc
	if(args->VirtualKey == VirtualKey::Escape)
		m_windowClosed = true;
}

void RubikCube::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void RubikCube::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// Insert your code here.

		deferral->Complete();
	});
}
 
void RubikCube::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new RubikCube();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}
