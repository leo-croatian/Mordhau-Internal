#include "pch.h"

#include "SDK.h"
#include "SDK/BasicTypes_Package.cpp"
#include "SDK/CoreUObject_Package.cpp"
#include "SDK/Engine_Package.cpp"
#include "SDK/Mordhau_Package.cpp"
#include "SDK/BP_MordhauCharacter_Package.cpp"
#include "SDK/BP_MordhauHUD_Package.cpp"

#include "Globals.hpp"


void OnDraw( );
void InitImGui()
{
	ImGui::CreateContext();

	auto io = ImGui::GetIO( );
	auto& style = ImGui::GetStyle( );

	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	style.WindowMinSize = ImVec2( 256, 300 );
	style.WindowTitleAlign = ImVec2( 0.5, 0.5 );
	style.FrameBorderSize = 1;
	style.ChildBorderSize = 1;
	style.WindowBorderSize = 1;
	style.WindowRounding = 0;
	style.FrameRounding = 0;
	style.ChildRounding = 0;
	style.Colors[ ImGuiCol_TitleBg ] = ImColor( 70, 70, 70 );
	style.Colors[ ImGuiCol_TitleBgActive ] = ImColor( 70, 70, 70 );
	style.Colors[ ImGuiCol_TitleBgCollapsed ] = ImColor( 70, 70, 70 );
	style.Colors[ ImGuiCol_WindowBg ] = ImColor( 25, 25, 25, 240 );
	style.Colors[ ImGuiCol_CheckMark ] = ImColor( 70, 70, 70 );
	style.Colors[ ImGuiCol_Border ] = ImColor( 70, 70, 70 );
	style.Colors[ ImGuiCol_Button ] = ImColor( 32, 32, 32 );
	style.Colors[ ImGuiCol_ButtonActive ] = ImColor( 42, 42, 42 );
	style.Colors[ ImGuiCol_ButtonHovered ] = ImColor( 42, 42, 42 );
	style.Colors[ ImGuiCol_ChildBg ] = ImColor( 45, 45, 45 );
	style.Colors[ ImGuiCol_FrameBg ] = ImColor( 32, 32, 32 );
	style.Colors[ ImGuiCol_FrameBgActive ] = ImColor( 42, 42, 42 );
	style.Colors[ ImGuiCol_FrameBgHovered ] = ImColor( 42, 42, 42 );
	style.Colors[ ImGuiCol_SliderGrab ] = ImColor( 255, 255, 255 );
	style.Colors[ ImGuiCol_SliderGrabActive ] = ImColor( 255, 255, 255 );

	io.Fonts->AddFontFromFileTTF( "C:\\Windows\\Fonts\\Tahoma.ttf", 14.0f );

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc( const HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam ) {

	if ( uMsg == WM_KEYUP && wParam == VK_HOME )
		Globals::Open ^= 1;

	if ( Globals::Open )
	{
		ImGui_ImplWin32_WndProcHandler( hWnd , uMsg , wParam , lParam );
		return true;
	}	

	return CallWindowProc( oWndProc , hWnd , uMsg , wParam , lParam );
}

namespace SDK
{
	CG::UWorld** m_world;
	CG::ULevel* m_persistence_level;
	CG::ULocalPlayer* local_player;
	uintptr_t Bones;
}

void InitSDK ( )
{
	uintptr_t base_address = (reinterpret_cast< uintptr_t >(GetModuleHandleA ( nullptr )));
	SDK::m_world = reinterpret_cast< CG::UWorld** >(base_address + 0x58CBEE0);
	CG::UObject::GObjects = reinterpret_cast< CG::TUObjectArray* >(base_address + 0x5795D70);
	CG::FName::GNames = reinterpret_cast< CG::FNamePool* >(base_address + 0x577D140);
	SDK::Bones = base_address + 0x2F8F690; // E8 ? ? ? ? 48 8B 47 30 F3 0F 10 45 - GetBoneMatrix
}

typedef CG::FMatrix* (__thiscall* _GetBoneMatrix)(CG::USkeletalMeshComponent* mesh , CG::FMatrix* result , int index);
CG::FVector GetBoneLocationByIndex ( CG::USkeletalMeshComponent* mesh , int index )
{
	if ( !mesh )
		return { 0.f, 0.f, 0.f };

	CG::FMatrix matrix{};

	_GetBoneMatrix fGetBoneMatrix = reinterpret_cast< _GetBoneMatrix >(SDK::Bones);
	fGetBoneMatrix ( mesh , &matrix , index );

	return matrix.WPlane;
}

float DistTo ( CG::FVector first , CG::FVector second )
{
	CG::FVector delta;
	delta.X = first.X - second.X;
	delta.Y = first.Y - second.Y;
	delta.Z = first.Z - second.Z;

	return sqrt ( delta.X * delta.X + delta.Y * delta.Y + delta.Z * delta.Z );
}

float Normalize ( float angle )
{
	float out = fmodf ( fmodf ( angle , 360.f ) + 360.f , 360.f );
	if ( out > 180.f )
		out -= 360.f;
	return out;
}


void Cheat ( )
{
	InitSDK ( );
	if ( (*SDK::m_world) == nullptr ) return;
	SDK::m_persistence_level = (*SDK::m_world)->PersistentLevel;
	SDK::local_player = (*SDK::m_world)->OwningGameInstance->LocalPlayers[ 0 ]; if ( (SDK::local_player) == nullptr ) return;
	auto* PlayerController = SDK::local_player->PlayerController; if ( (PlayerController) == nullptr ) return;
	auto* PlayerCameraManager = PlayerController->PlayerCameraManager;
	auto AcknowledgedPawn = PlayerController->AcknowledgedPawn; if ( (AcknowledgedPawn) == nullptr ) return;
	auto* RootComponent = AcknowledgedPawn->RootComponent; if ( (RootComponent) == nullptr ) return;
	auto GameState = (*SDK::m_world)->GameState; if ( (GameState) == nullptr ) return;
	auto PlayerArray = GameState->PlayerArray;

	for ( int i = 0; i < PlayerArray.Count ( ); i++ ) {
		auto Pawn = PlayerArray[ i ]->PawnPrivate; if ( (Pawn) == nullptr ) continue;
		auto ActorRootComponent = Pawn->RootComponent; if ( (ActorRootComponent) == nullptr ) continue;
		CG::AMordhauCharacter* Character = static_cast< CG::AMordhauCharacter* >(Pawn);
		auto MetabolismPrarm = Character->bIsDead; if ( MetabolismPrarm == 1 ) continue;
		auto hehe = static_cast< CG::UMordhauGameInstance* >((*SDK::m_world)->OwningGameInstance);
		CG::FVector RootBone;
		auto local_player_controller = static_cast< CG::AMordhauPlayerController* >(SDK::local_player->PlayerController);
		auto Player = static_cast< CG::AMordhauPlayerController* >(PlayerController);
		auto adv_char = static_cast< CG::AAdvancedCharacter* >(Character);
		auto MordhauPlayerController = static_cast< CG::AMordhauPlayerController* >(PlayerController);
		auto local_mordhau_character = static_cast< CG::ABP_MordhauCharacter_C* >(PlayerController->AcknowledgedPawn);
		if ( Character->IsLocallyControlled ( ) ) continue;
		CG::FVector head = GetBoneLocationByIndex ( Character->Mesh , 68 );

		CG::FVector2D headScreen;

		local_mordhau_character->bIsUnflinchable = true;

		if ( Globals::Misc::TurnCap )
		{
			adv_char->TurnRateCap = -1;
			adv_char->LookUpRateCap = -1;
		}
		if ( Globals::Misc::SuperSprint )
		{
			local_mordhau_character->StartSupersprint ( );
		}
		if ( Globals::Misc::VoiceSpam )
		{
			local_mordhau_character->StartScreaming ( );
		}
		else if ( !Globals::Misc::VoiceSpam )
		{
			local_mordhau_character->StopScreaming ( );
		}
		if ( Globals::Misc::InstantSprintRegen )
		{
			local_mordhau_character->Stamina = 1000;
		}
		if ( Globals::Misc::MakeAdmin )
		{
			local_mordhau_character->SetOwner ( Player );
		}

		if ( Globals::Visuals::DisplayInfo )
		{
			for ( int i = 0; i <= 100; i++ )
			{
				CG::FVector curbone = GetBoneLocationByIndex ( Character->Mesh , i );
				CG::FVector2D ok;

				if ( !PlayerController->ProjectWorldLocationToScreen ( curbone , &ok , false ) );
				ImGui::GetBackgroundDrawList ( )->AddText ( ImVec2 ( ok.X , ok.Y ) , ImColor ( 255 , 255 , 255 , 255 ) , (std::to_string ( i ).c_str ( )) );
			}
		}
		ImDrawList* drawList = ImGui::GetBackgroundDrawList ( );

		if ( Globals::Visuals::DisplayNames )
		{
			CG::FVector2D screen2;

			CG::FVector rootBone = GetBoneLocationByIndex ( Character->Mesh , 0 );
			CG::FVector head = GetBoneLocationByIndex ( Character->Mesh , 28 );
			CG::FVector2D rootScreen;
			CG::FVector2D headScreen;

			float top = headScreen.Y;
			float bottom = rootScreen.Y;
			float width = abs ( top - bottom ) / 2.f;
			float left = rootScreen.X - width / 2.f;
			float right = rootScreen.X + width / 2.f;

			ImDrawList* drawList2 = ImGui::GetBackgroundDrawList ( );
			CG::FString ueName = local_mordhau_character->PlayerState->PlayerNamePrivate;

			std::string name = std::string ( ueName.ToString ( ) );
			ImVec2 textSize = ImGui::CalcTextSize ( name.c_str ( ) );

			if ( PlayerController->ProjectWorldLocationToScreen ( head , &screen2 , false ) )
			{
				if ( ueName.IsValid ( ) )
				{
					drawList->AddText ( ImVec2 ( screen2.X , screen2.Y - 14 ) , ImColor ( 255 , 0 , 0 , 255 ) , name.c_str ( ) );
				}
			}
		}
	}
}

HRESULT __stdcall hkPresent( IDXGISwapChain* pSwapChain , UINT SyncInterval , UINT Flags )
{
	static bool Init = false;
	if ( !Init )
	{
		if ( SUCCEEDED( pSwapChain->GetDevice( __uuidof( ID3D11Device ) , (void**)&pDevice ) ) )
		{
			pDevice->GetImmediateContext( &pContext );
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc( &sd );
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer( 0 , __uuidof( ID3D11Texture2D ) , (LPVOID*)&pBackBuffer );
			pDevice->CreateRenderTargetView( pBackBuffer , NULL , &mainRenderTargetView );
			pBackBuffer->Release( );
			oWndProc = (WNDPROC)SetWindowLongPtr( window , GWLP_WNDPROC , (LONG_PTR)WndProc );
			InitImGui( );
			Init = true;
		}

		else
			return oPresent( pSwapChain , SyncInterval , Flags );
	}

	ImGui_ImplDX11_NewFrame( );
	ImGui_ImplWin32_NewFrame( );
	ImGui::NewFrame( );
	Cheat ( );

	ImGui::GetIO( ).MouseDrawCursor = Globals::Open;
	OnDraw( );
	
	ImGui::Render( );

	pContext->OMSetRenderTargets( 1 , &mainRenderTargetView , NULL );
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );
	return oPresent( pSwapChain , SyncInterval , Flags );
}

void OnDraw()
{
	static auto Slider = [ & ] ( const char* label, float* value, float min, float max, float width = ImGui::GetContentRegionAvail( ).x )
	{
		ImGui::PushID( label );
		ImGui::PushItemWidth( width );
		ImGui::SliderFloat( _( "##CustomSliderF_" ), value, min, max );
		ImGui::PopItemWidth( );
		ImGui::PopID( );
		ImGui::Spacing( );
	};

	static auto Checkbox = [ & ] ( const char* label, bool* value )
	{
		ImGui::PushID( label );
		ImGui::Checkbox( label, value );
		ImGui::PopID( );
		ImGui::Spacing( );
	};

	if ( Globals::Open ) {
		ImGui::Begin( _( "D3D11 Hook" ) , 0 , ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize );
		{
			if ( ImGui::Button( _( "Visuals" ) , ImVec2( ImGui::GetContentRegionAvail( ).x / 4 , 29 ) ) )
				Globals::Tab = 0;

			ImGui::SameLine( );

			if ( ImGui::Button( _( "World" ) , ImVec2( ImGui::GetContentRegionAvail( ).x / 3 , 29 ) ) )
				Globals::Tab = 1;

			ImGui::SameLine( );

			if ( ImGui::Button( _( "Other" ) , ImVec2( ImGui::GetContentRegionAvail( ).x / 2 , 29 ) ) )
				Globals::Tab = 2;

			ImGui::SameLine( );

			if ( ImGui::Button( _( "Settings" ) , ImVec2( ImGui::GetContentRegionAvail( ).x / 1 , 29 ) ) )
				Globals::Tab = 3;

			ImGui::Spacing( );
			ImGui::Separator( );

			switch ( Globals::Tab )
			{
			case 0: // Visuals
				Checkbox ( _ ( "ESP Enabled" ) , &Globals::Visuals::Enabled );
				Checkbox ( _ ( "Boxes" ) , &Globals::Visuals::Boxes );
				Checkbox ( _ ( "Filled Boxes" ) , &Globals::Visuals::FilledBoxes );
				Checkbox ( _ ( "Snaplines" ) , &Globals::Visuals::Snaplines );
				Checkbox ( _ ( "Display Info" ) , &Globals::Visuals::DisplayInfo );
				Checkbox ( _ ( "Display Health" ) , &Globals::Visuals::DisplayHealth );
				Checkbox ( _ ( "Display Names" ) , &Globals::Visuals::DisplayNames );
				break;

			case 1: // World
				
				break;

			case 2: // Other
				Checkbox ( _ ( "Disable Turn Cap" ) , &Globals::Misc::TurnCap );
				Checkbox ( _ ( "Super Sprint" ) , &Globals::Misc::SuperSprint );
				Checkbox ( _ ( "Voice Spam" ) , &Globals::Misc::VoiceSpam );
				Checkbox ( _ ( "Instant Sprint Regen" ) , &Globals::Misc::InstantSprintRegen );
				Checkbox ( _ ( "Make Admin" ) , &Globals::Misc::MakeAdmin );
				break;

			case 3: // Settings
				if ( ImGui::Button( _( "Unhook" ) ) )
					Globals::IsClosing = true;
				break;
			}
		}
		ImGui::End( );
	}
}

int MainThread()
{
#if CONSOLE_LOGGING
	AllocConsole( );
	freopen_s( ( FILE** )stdout, "CONOUT$", "w", stdout );
#endif

	Logger ( "Injected at: %s", __TIME__ );
	Logger ( "Module Instance: 0x%p", reinterpret_cast<void*>(Globals::DllInstance) );

	static bool Done = false;

	while ( !Done )
	{
		if ( kiero::init( kiero::RenderType::D3D11 ) == kiero::Status::Success )
		{
			kiero::bind( 8, ( void** )&oPresent, hkPresent );
			Done = true;
		}
	}

	while ( true )
	{
		if ( ( GetAsyncKeyState( VK_DELETE ) & 1 ) || Globals::IsClosing )
			break;

		Sleep( 1000 );
	}

	Logger ( "Unhooked at: %s", __TIME__ );

	Globals::IsClosing = false;
	Globals::Open = false;

	kiero::unbind( 8 );
	kiero::shutdown( );

	FreeLibrary ( (HMODULE)Globals::DllInstance );
	return TRUE;
}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, void* )
{
	Globals::DllInstance = hModule;

	if ( dwReason == DLL_PROCESS_ATTACH )
		CloseHandle( CreateThread( 0 , 0 , reinterpret_cast<PTHREAD_START_ROUTINE>( MainThread ) , 0 , 0 , 0 ) );

	return TRUE;
}
