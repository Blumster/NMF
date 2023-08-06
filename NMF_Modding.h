#pragma once

#ifndef NMF_VERSION
#include "NMF.h"
#endif

#ifdef NMF_USE_MODDING

#include <map>
#include <stdint.h>
#include <string>

namespace NMF
{
    class ModBase;

    class ModuleBase
    {
    public:
        ModuleBase(ModBase* owner, const char* moduleName)
            : Owner(owner), ModuleName(moduleName)
        {
        }
        virtual ~ModuleBase() {}

        virtual void OnAttach() {}
        virtual void OnBeforeStart(const char* commandLine) {}
        virtual WndProcHandleResult WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return WndProcHandleResult::Continue; }

#ifdef NMF_USE_IMGUI
        virtual void OnImGuiDraw(ImGuiDrawTarget target) {}
#endif

        const char* GetName() const { return ModuleName; }
        ModBase* GetOwner() const { return Owner; }

    private:
        ModBase* Owner;
        const char* ModuleName;
    };

    class ModBase
    {
    public:
        ModBase(const char* modName)
            : ModName(modName)
        {
        }
        virtual ~ModBase() {}

        virtual void OnAttach()
        {
            for (const auto& module : Modules)
                module.second->OnAttach();
        }

        virtual void OnBeforeStart(const char* commandLine)
        {
            for (const auto& module : Modules)
                module.second->OnBeforeStart(commandLine);
        }

        virtual WndProcHandleResult WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            WndProcHandleResult result;

            for (const auto& module : Modules)
            {
                result = module.second->WndProc(hWnd, uMsg, wParam, lParam);
                if (result != WndProcHandleResult::Continue)
                    return result;
        }

            return WndProcHandleResult::Continue;
        }

#ifdef NMF_USE_IMGUI
        virtual void OnImGuiDraw(ImGuiDrawTarget target)
        {
            for (const auto& module : Modules)
                module.second->OnImGuiDraw(target);
        }
#endif

        const char* GetName() const { return ModName; }

        virtual bool RegisterModule(ModuleBase* module)
        {
#ifdef NMF_USE_LOGGING
            Logger.Log(LogSeverity::Debug, "Registering module: %s in mod: %s!", module->GetName(), GetName());
#endif

            std::string moduleName(module->GetName());

            auto itr = Modules.find(moduleName);
            if (itr != Modules.end())
            {
#ifdef NMF_USE_LOGGING
                Logger.Log(LogSeverity::Error, "A module under the name %s has already been registered! Skipping...", module->GetName());
#endif

                return false;
            }

            Modules[moduleName] = module;

#ifdef NMF_USE_LOGGING
            Logger.Log(LogSeverity::Debug, "Module: %s in mod: %s has been successfully registered!", module->GetName(), GetName());
#endif

            return true;
        }

        virtual ModuleBase* GetModule(const char* moduleNameRaw) const
        {
            std::string moduleName{ moduleNameRaw };

            auto itr = Modules.find(moduleName);
            if (itr != Modules.end())
                return itr->second;

            return nullptr;
        }

    private:
        const char* ModName;

#ifdef NMF_USE_LOGGING
        static Logger Logger;
#endif

#pragma warning(disable: 4251)
        std::map<std::string, ModuleBase*> Modules;
#pragma warning(default: 4251)
    };

    class ModManagerBase
    {
    public:
        NMF_EXPORT static ModManagerBase* Instance;

        ModManagerBase()
        {
#ifdef NMF_USE_IMGUI
            if (Instance == nullptr)
                Instance = this;
#endif
        }

        virtual ~ModManagerBase() {}

        virtual bool RegisterMod(ModBase* mod)
        {
            std::string modName(mod->GetName());

            auto itr = Mods.find(modName);
            if (itr != Mods.end())
                return false;

            Mods[modName] = mod;

            mod->OnAttach();

            return true;
        }

        virtual ModBase* GetMod(const char* modNameRaw)
        {
            std::string modName{ modNameRaw };

            auto itr = Mods.find(modName);
            if (itr != Mods.end())
                return itr->second;

            return nullptr;
        }

        virtual void BeforeStart(HINSTANCE hInstance, HINSTANCE hPrevInstance, const char* lpCmdLine, int nShowCmd)
        {
            for (const auto& mod : Mods)
                mod.second->OnBeforeStart(lpCmdLine);
        }

        virtual WndProcHandleResult WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            WndProcHandleResult result;

            for (const auto& mod : Mods)
            {
                result = mod.second->WndProc(hWnd, uMsg, wParam, lParam);

                if (result != WndProcHandleResult::Continue)
                    return result;
            }

            return WndProcHandleResult::Continue;
        }

        void ForEach(std::function<void(ModBase*)> func)
        {
            for (const auto& mod : Mods)
                func(mod.second);
        }

#ifdef NMF_USE_IMGUI
        virtual void OnImGuiDraw(ImGuiDrawTarget target)
        {
            for (const auto& mod : Mods)
                mod.second->OnImGuiDraw(target);
        }
#endif

    protected:
#pragma warning(disable: 4251)
        std::map<std::string, ModBase*> Mods;
#pragma warning(default: 4251)
    };
}

#endif
