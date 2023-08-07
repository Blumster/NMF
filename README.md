# NMF - Native Modding Framework

NMF is a collection of utility features that help with creating DLLs that can be injected into games to extend its capabilities. The goal is to easily reuse these core features in any game.

NMF only supports **Windows**, both **32 and 64 bits**.

## Features

* Memory control
  * Patch handling
  * Hook handling
  * Global value/pointer/etc. wrappers
  * Game function wrappers
  * Memory manager to manage patches and hooks
  * Dynamic memory address handling
* Logging
  * Console log sink
  * File log sink
  * Scope based logging
* Mod system base
  * Modules in mods for better code organization
  * Mod manager to propagate events into mods and modules
* ImGui
  * Toggleable overlay UI with ImGui (pressing F8)
  * Returning the control to the game, but leaving the UI open (pressing F9) *Requires `NMF_EXTERN_SET_GAME_FOCUS`!*
  * Configurable console windows in the overlay UI
* RTTI
  * Helpers to use the game's own RTTI for `typeid` and `dynamic_cast`

### Planned features

* Lua scripting engine
* Utility to define and validate classes exposed from the game
* ImGui support for: DX10, DX11, DX12
* ImGui console log sink
* Pattern based memory search
* Input handling (DirectInput8, XInput, WinProc, etc...)

## Usage

1. Add NMF as a submodule to your repo
1. Add all header and source files to your DLL project
1. Create a user config header (e.g.: `NMF_Config.h`) in your project
1. Define all the [features](#feature-defines) you want in your user config header
1. Add the path to the user config header in your preprocessor (`NMF_USER_CONFIG="..\path\to\your\code\from\NMF\submodule\NMF_Config.h"`) *NOTE: Do it globally, not only in a header file!*
1. Create a common header/pch/something that includes `NMF.h` from the submodule and an optional `using namespace NMF`
1. Use NMF in your codebase!

## Feature defines

* `NMF_USE_IMGUI` - Turns on ImGui
  * `NMF_IMGUI_DX9` - Enables DirectX9 support
    * `NMF_IMGUI_DX9_HOOK_VIRTUAL` - Hook `EndScene` and `Reset` virtual functions automatically for rendering the overlay UI
  * `NMF_IMGUI_POP_OUT` - Enables popping out the overlay UI to a new window, so it won't hide parts the game
  * `NMF_IMGUI_DEMO` - Enables the ImGui demo window for widget reference
  * `NMF_EXTERN_SET_GAME_FOCUS` - Defines the existence of the `SetGameFocus` external function, to further extend the capabilities on the ImGui overlay UI
* `NMF_USE_LOGGING` - Turns on logging
* `NMF_USE_MODDING` - Turns on modding base
* `NMF_USE_RTTI` - Enables `RTTI::TypeId` and `RTTI::DynamicCast`
* `NMF_USER_CONFIG` - The path of the user config header
* `NMF_STATIC_BASE` - The static base of the executable's code
* `NMF_EXPORT` - DLL export or import, if external DLL mods are used
* `NMF_EXTENDED_EXPORT` - DLL export or import to some more symbols

## Examples

**NOTE: The memory addresses are arbitrary, every different game will have different addresses!**

### Setting up NMF

You can create an SDK class or just initialize NMF from the DllMain.

```cpp
#include <Windows.h>

#include "path\NMF.h"

using namespace NMF;

class MyOwnModManager : public ModManagerBase
{
public:
    void Setup()
    {
        // Something
    }
    void Teardown()
    {
        // Something
    }
}

MyOwnModManager* ModManagerInstance;

Hook* BeforeStartHook;
Hook* DeviceCreateHook;

void SomeHookBeforeTheGameIsRan(HINSTANCE hInstance, HINSTANCE hPrevInstance, const char* lpCmdLine, int nShowCmd);
void SomeCallbackAfterD3D9IsSetup(HWND hwnd, IDirect3D9* d3d, IDirect3DDevice9* device);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            // Restore default working after being injected.
            // If you use external mods, this might not be the place to put this
            DetourRestoreAfterWith();

            // Sets up logging (if it was compiled with NMF_USE_LOGGING defined) to file <working dir>/logs/<timestamp>_log.txt and a new console window, displaying every log message. (Info, Debug, Error, etc...)
            LogManager::Setup(LogDestination::File | LogDestination::Console, LogFilter::All);

            // Initializes NFM's MemoryManager to keep track of patches and hooks
            MemoryManager::Setup();

            // Enables mod system (if it was compiled with NMF_USE_MODDING defined)
            // If external mod support is needed, you need to create a new mod manager, inheriting from the NMF::ModManagerBase
            // The ModManagerBase instance pointer is stored at NMF::ModManagerBase::Instance for later use, if not stored otherwise
            ModManagerInstance = new MyOwnModManager();
            ModManagerInstance->Setup();

            // The hook functions must match the function that is being hooked
            // (return value, parameters, calling convention, etc...)
            // Create hook that runs before the game is started to init ImGuiManager
            BeforeStartHook = MemoryManager::CreateAndApplyHook(0x4FEDCB_addr, &SomeHookBeforeTheGameIsRan);

            // Create hook that runs after D3D9 device creation to enable ImGuiManager's rendering
            DeviceCreateHook = MemoryManager::CreateAndApplyHook(0x4BCDEF_addr, &SomeCallbackAfterD3D9IsSetup);
            break;
        }

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            ImGuiManager::Teardown();

            ModManagerInstance->Teardown();

            MemoryManager::Teardown();

            LogManager::TearDown();
            break;
    }

    return TRUE;
}

void SomeHookBeforeTheGameIsRan(HINSTANCE hInstance, HINSTANCE hPrevInstance, const char* lpCmdLine, int nShowCmd)
{
    // Sets up ImGuiManager to be able to create external window for the pop out feature
    ImGuiManager::Setup(hInstance, nShowCmd);

    // Call the original (currently a trambouline) function to execute the original code
    BeforeStartHook->GetOriginalFunction<decltype(&SomeHookBeforeTheGameIsRan)>()(hInstance, hPrevInstance, lpCmdLine, nShowCmd);

    // Hook can be disabled, as it is no longer used
    BeforeStartHook->Remove();
}

void SomeCallbackAfterD3D9IsSetup(HWND hwnd, IDirect3D9* d3d, IDirect3DDevice9* device)
{
    // Sets up the (in this case) DX9 layer of ImGuiManager to be able to render the overlay UI
    ImGuiManager::SetupD3D(hwnd, d3d, d3dDevice);

    DeviceCreateHook->GetOriginalFunction<decltype(&SomeCallbackAfterD3D9IsSetup)>()(hwnd, d3d, d3dDevice);

    DeviceCreateHook->Remove();
}

// This is *usually* needed for Microsoft Detours
extern "C" __declspec(dllexport) int MySDK_DetoursDummyExportFunction()
{
    return 0;
}
```

### Memory management

The `MemoryManager` handles creating patches and hooks and tracks them. Hooking the same function or patching the same area of memory is not allowed in the manager.

#### Patching

```cpp
void PatchSomething()
{
    // Replace 2 bytes of memory
    Patch* patch = MemoryManager::CreateAndApplyPatch(0x4AAAAA_addr, { 0x4F, 0x3B });

    // Nop out 10 bytes of memory
    Patch* nopPatch = MemoryManager::NopMemory(0x4ABBAA_addr, 10);
}
```

#### Hooking

```cpp
Hook* hook1;
Hook* hook2;

using Hook1FuncType = void(int);

void HookFunc(int param)
{
    // Do something here, BEFORE running the original function

    // Call the original function OR calling the original function can be skipped
    hook1->GetOriginalFunction<decltype(&HookFunc)>()(param);
    // or
    hook1->GetOriginalFunction<Hook1FuncType*>()(param);

    // Do something here, AFTER running the original function
}

void __fastcall HookFuncForThiscall(void* thisPtr, void* edx, int param)
{
    // Thiscall functions cannot be represented properly, but can be emulated with __fastcall with having a second parameter for edx and passing it along to the original call

    // NOTE: 64 bit fastcalls are different! Also 64 bit doesn't usually use __thiscall

    hook2->GetOriginalFunction<decltype(&HookFuncForThiscall)>()(thisPtr, edx, param);
}

void HookSomething()
{
    // Creates and installs a hook
    hook1 = MemoryManager::CreateAndApplyHook(0x4EEEEE_addr, &HookFunc);

    // Creates and installs a hook for a __thiscall function
    hook2 = MemoryManager::CreateAndApplyHook(0x4FFFFF_addr, &HookFuncForThiscall);
}

```

### Exposing classes, values and functions

```cpp
#include "NMF.h"

using namespace NMF;

// The representation of the Player class that is used in the game
// The size and layout of the class must match the one in the game otherwise invalid reads and writes will happen
class Player
{
public:
    char unknown_or_padding_bytes[0x40];
    const char* Name;
    char other_unknown_or_padding_bytes[0x40];

    static ThisGameFunction<void, Player*, int> AddMoney;
    static GlobalPtr<Player> Instance;

    void Optional_Helper_Function_AddMoney(int money)
    {
        AddMoney(this, money);
    }
};

// Static asserts can be used to validate the class layout that is generated by the compiler
static_assert(offsetof(Player, unknown_or_padding_bytes) == 0x00);
static_assert(offsetof(Player, Name) == 0x40, "Player's Name is at a wrong offset!");
static_assert(offsetof(Player, other_unknown_or_padding_bytes) == 0x44);
static_assert(sizeof(Player) == 0x84, "Player class has different size than expected!");

// The addresses are the static addresses that were reverse engineered out of the client
// This leads to easier backtracking, if you want to jump to an address you copy out of the source code
// The addresses are translated the following way to defeat ASLR:
// <Final Address> = (<Address> - <Static address base>) + <Dynamic address base>
// Assuming the client is at static address 0x400000 (32 bit default)
// Assuming the process is put at 0x1000000 dynamic base address
// The real address will be:
// (0x412345 - 0x400000) + 0x1000000 = 0x1012345
// Usually 32 bit games doesn't use ASLR, but 64 bit games use it as default
inline ThisGameFunction<void, Player*, int> Player::AddMoney{ 0x41A2B3_addr };
inline GlobalPtr<Player> Player::Instance{ 0x412345_addr };

void AddMoneyToPlayer()
{
    // The static values can be used for calling the functions in the client
    Player::AddMoney(Player::Instance, 100);

    // or helper functions can be introduced to prettify the code
    Player::Instance->Optional_Helper_Function_AddMoney(100);

    // Member values can be accessed easily
    printf("%s got money!", Player::Instance->Name);
}

GlobalValue<int> NPCCount{ 0x4ABCDE_addr };

void ManualSpawnNPC()
{
    // Code to spawn an NPC

    // You can manipulate the global values easily with the help of the wrappers
    NPCCount += 1;
}
```

### User config

Example `NMF_Config.h`:

```cpp
#pragma once

// Expose NMF capabilities from the DLL to external DLL mods
#ifdef MY_DLL_BUILD
#define NMF_EXPORT __declspec(dllexport)
#define NMF_EXTENDED_EXPORT __declspec(dllexport)
#else
#define NMF_EXPORT __declspec(dllimport)
#define NMF_EXTENDED_EXPORT __declspec(dllimport)
#endif

// Turn on ImGui overlay UI
#define NMF_USE_IMGUI

// With DX9 support
#define NMF_IMGUI_DX9

// Automatically hook DX9 virtual functions for Rendering
// Otherwise manual calls to ImGuiManager::Render is needed
#define NMF_IMGUI_DX9_HOOK_VIRTUAL

// Enable popping out the overlay UI to a separate window
#define NMF_IMGUI_POP_OUT

// Enable ImGui demo
#define NMF_IMGUI_DEMO

// Enable logging (in ImGui, modding, memory) and enable creating loggers that use the same logging system (to file, to console, etc...)
#define NMF_USE_LOGGING

// Enable modding system base
#define NMF_USE_MODDING

// Enables RTTI functions
#define NMF_USE_RTTI

// Enables extra capabilities in the ImGui overlay UI
#define NMF_EXTERN_SET_GAME_FOCUS
```

### Extern functions in NMF

Defining external functions for NMF to use is easy. The function must be defined in your project in a source file in the namespace `NMF::External` and the proper define must be set in the user config header.

Example for `NMF_EXTERN_SET_GAME_FOCUS` implementing `SetGameFocus(bool focus)`:

```cpp
#include <your_solution_header.h>

// This namespace contains the external functions
namespace NMF::External
{
    void SetGameFocus(bool focus)
    {
        // Your solution to gain/lose focus of the window and maybe showing a cursor, so when opening ImGui a new cursor is created that works properly
        YourSolution::SetGameFocus(focus);
    }
}
```

### Using the game's RTTI

```cpp
#include "NMF.h"

// These classes were recovered from the game, they have their RTTI data available
class A
{
public:
    static void* TypeDescriptor;
}

class B : public A
{
public:
    static void* TypeDescriptor;
}

inline void* A::TypeDescriptor{ 0x1000000_addr };
inline void* B::TypeDescriptor{ 0x1000100_addr };

// Using dynamic_cast and typeid would generate calls to wrong RTTI data, so the RTTI class is there to fill the gap.
void Downcast(A* baseObj)
{
    // This could be bad and point to an invalid B!
    B* badPtr = (B*)baseObj;

    // This would use the RTTI data of OUR classes, not the RTTI data in the game
    B* emptyPtr = dynamic_cast<B*>(baseObj);

    // This is a correct dynamic cast, as it uses A::TypeDescriptor and B::TypeDescriptor to do the actual dynamic casting
    B* goodPtr = RTTI::DynamicCast(baseObj);

    // The type info can be requested from the game
    const std::type_info* infoPtr = RTTI::TypeId(baseObj);

    printf(infoPtr->name());
}
```

### Logging

```cpp
#include "NMF.h"

Logger MyLogger{ "MyScope1", "MyScope2" };

void LogMyMessage()
{
    // This would generate the following log line on the sinks:
    // [TIMESTAMP][Info][MyScope1][MyScope2]: Hello world!
    MyLogger.Log(LogSeverity::Info, "Hello world!");
}
```

### Modding

```cpp
#include "NMF.h"

using namespace NMF;

MyModManager* modManagerInstance;
Hook* MyEventHook;

class MyMod : public ModBase
{
public:
    // Define a new "event" where all mod should be notified
    virtual void AfterSomeEventInMyGame(int param)
    {
        // React to it
    }
}

void MyEventHookFunc(int param)
{
    MyEventHook->GetOriginalFunction<void(int)>()(param);

    modManagerInstance->AfterSomeEventInMyGame(param);
}

class MyModManager : public ModManagerBase
{
public:
    void Setup()
    {
        MyEventHook = MemoryManager::CreateAndApplyHook(0x432101_addr, &MyEventHookFunc);
    }

    virtual void AfterSomeEventInMyGame(int param)
    {
        // Run the lambda for each mod
        ForEach([param](ModBase* mod)
        {
            // Check if it's a MyMod instance then propagate the "event"
            if (MyMod* myMod = dynamic_cast<MyMod*>(mod))
                myMod->AfterSomeEventInMyGame(param);
        });
    }
}

modManagerInstance = new MyModmanager();
modManagerInstance->Setup();
```

### ImGui overlay UI

```cpp
#include "NMF.h"

using namespace NMF;

class MyMod : public ModBase
{
public:
    virtual void OnImGuiDraw(ImGuiDrawTarget target)
    {
        // The menu item for the main menu can be drawn here
        if (target == ImGuiDrawTarget::MainMenu)
        {
            if (ImGui::MenuItem("My window"))
                Shown = !Shown;

            return;
        }

        // The window or other widget can be drawn here
        if (target != ImGuiDrawTarget::General)
        {
            return;
        }

        // You can use ImGui function to build a UI
        ImGui::Begin("My ImGui window", &Shown);
        ImGui::Button("Click");
        ImGui::End();
    }

private:
    bool Shown;
}

auto modManager = new ModManagerBase();
modManager->RegisterMod(new MyMod);
```
