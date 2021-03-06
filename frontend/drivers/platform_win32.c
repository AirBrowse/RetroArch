/* RetroArch - A frontend for libretro.
 * Copyright (C) 2011-2015 - Daniel De Matteis
 *
 * RetroArch is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Found-
 * ation, either version 3 of the License, or (at your option) any later version.
 *
 * RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 * * You should have received a copy of the GNU General Public License along with RetroArch.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef _XBOX
#include <xtl.h>
#else
#include <windows.h>
#endif
#include <retro_miscellaneous.h>
#include "../frontend_driver.h"
#include <dynamic/dylib.h>
#include "../../general.h"

#include <stdint.h>
#include <boolean.h>
#include <stddef.h>
#include <string.h>

#if defined(_WIN32) && !defined(_XBOX)
/* We only load this library once, so we let it be 
 * unloaded at application shutdown, since unloading 
 * it early seems to cause issues on some systems.
 */

static dylib_t dwmlib;

static bool dwm_composition_disabled;

static void gfx_dwm_shutdown(void)
{
   if (dwmlib)
      dylib_close(dwmlib);
   dwmlib = NULL;
}

static bool gfx_init_dwm(void)
{
   static bool inited = false;

   if (inited)
      return true;

   dwmlib = dylib_load("dwmapi.dll");
   if (!dwmlib)
   {
      RARCH_LOG("Did not find dwmapi.dll.\n");
      return false;
   }
   atexit(gfx_dwm_shutdown);

   HRESULT (WINAPI *mmcss)(BOOL) = 
      (HRESULT (WINAPI*)(BOOL))dylib_proc(dwmlib, "DwmEnableMMCSS");
   if (mmcss)
   {
      RARCH_LOG("Setting multimedia scheduling for DWM.\n");
      mmcss(TRUE);
   }

   inited = true;
   return true;
}

static void gfx_set_dwm(void)
{
   HRESULT ret;
   settings_t *settings = config_get_ptr();

   if (!gfx_init_dwm())
      return;

   if (settings->video.disable_composition == dwm_composition_disabled)
      return;

   HRESULT (WINAPI *composition_enable)(UINT) = 
      (HRESULT (WINAPI*)(UINT))dylib_proc(dwmlib, "DwmEnableComposition");
   if (!composition_enable)
   {
      RARCH_ERR("Did not find DwmEnableComposition ...\n");
      return;
   }

   ret = composition_enable(!settings->video.disable_composition);
   if (FAILED(ret))
      RARCH_ERR("Failed to set composition state ...\n");
   dwm_composition_disabled = settings->video.disable_composition;
}
#endif

static void frontend_win32_get_os(char *name, size_t sizeof_name, int *major, int *minor)
{
	uint32_t version = GetVersion();

	*major   = (DWORD)(LOBYTE(LOWORD(version)));
	*minor   = (DWORD)(HIBYTE(LOWORD(version)));

   switch (*major)
   {
      case 6:
         switch (*minor)
         {
            case 3:
               strlcpy(name, "Windows 8.1", sizeof_name);
               break;
            case 2:
               strlcpy(name, "Windows 8", sizeof_name);
               break;
            case 1:
               strlcpy(name, "Windows 7/2008 R2", sizeof_name);
               break;
            case 0:
               strlcpy(name, "Windows Vista/2008", sizeof_name);
               break;
            default:
               break;
         }
         break;
      case 5:
         switch (*minor)
         {
            case 2:
               strlcpy(name, "Windows 2003", sizeof_name);
               break;
            case 1:
               strlcpy(name, "Windows XP", sizeof_name);
               break;
            case 0:
               strlcpy(name, "Windows 2000", sizeof_name);
               break;
         }
         break;
      case 4:
         switch (*minor)
         {
            case 0:
               strlcpy(name, "Windows NT 4.0", sizeof_name);
               break;
            case 90:
               strlcpy(name, "Windows ME", sizeof_name);
               break;
            case 10:
               strlcpy(name, "Windows 98", sizeof_name);
               break;
         }
         break;
      default:
         break;
   }
}

static void frontend_win32_init(void *data)
{
	typedef BOOL (WINAPI *isProcessDPIAwareProc)();
	typedef BOOL (WINAPI *setProcessDPIAwareProc)();
	HMODULE handle                         = GetModuleHandle(TEXT("User32.dll"));
	isProcessDPIAwareProc  isDPIAwareProc  = (isProcessDPIAwareProc)dylib_proc(handle, "IsProcessDPIAware");
	setProcessDPIAwareProc setDPIAwareProc = (setProcessDPIAwareProc)dylib_proc(handle, "SetProcessDPIAware");

	if (isDPIAwareProc)
	{
		if (!isDPIAwareProc())
		{
			if (setDPIAwareProc)
				setDPIAwareProc();
		}
	}
   
   gfx_set_dwm();
}

enum frontend_powerstate frontend_win32_get_powerstate(int *seconds, int *percent)
{
    SYSTEM_POWER_STATUS status;
	enum frontend_powerstate ret = FRONTEND_POWERSTATE_NONE;

	if (!GetSystemPowerStatus(&status))
		return ret;

	if (status.BatteryFlag == 0xFF)
		ret = FRONTEND_POWERSTATE_NONE;
	if (status.BatteryFlag & (1 << 7))
		ret = FRONTEND_POWERSTATE_NO_SOURCE;
	else if (status.BatteryFlag & (1 << 3))
		ret = FRONTEND_POWERSTATE_CHARGING;
	else if (status.ACLineStatus == 1)
		ret = FRONTEND_POWERSTATE_CHARGED;
	else
		ret = FRONTEND_POWERSTATE_ON_POWER_SOURCE;

	*percent  = (int)status.BatteryLifePercent;
	*seconds  = (int)status.BatteryLifeTime;

	return ret;
}

enum frontend_architecture frontend_win32_get_architecture(void)
{
   /* stub */
   return FRONTEND_ARCH_NONE;
}

const frontend_ctx_driver_t frontend_ctx_win32 = {
   NULL,						   /* environment_get */
   frontend_win32_init,
   NULL,                           /* deinit */
   NULL,                           /* exitspawn */
   NULL,                           /* process_args */
   NULL,                           /* exec */
   NULL,                           /* set_fork */
   NULL,                           /* shutdown */
   NULL,                           /* get_name */
   frontend_win32_get_os,
   NULL,                           /* get_rating */
   NULL,                           /* load_content */
   frontend_win32_get_architecture,
   frontend_win32_get_powerstate,
   "win32",
};
