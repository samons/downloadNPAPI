#include <npapi.h>
#include "npn_api.h"
#include "globals_glue.h"

namespace glue {
namespace globals {

void SetLastError(NPP npp, const char *error) {
}

}
}

extern "C" {
  NPError OSCALL NP_GetEntryPoints(NPPluginFuncs *pluginFuncs) {
    pluginFuncs->version = 11;
    pluginFuncs->size = sizeof(*pluginFuncs);
    pluginFuncs->newp = NPP_New;
    pluginFuncs->destroy = NPP_Destroy;
    pluginFuncs->setwindow = NPP_SetWindow;
    pluginFuncs->newstream = NPP_NewStream;
    pluginFuncs->destroystream = NPP_DestroyStream;
    pluginFuncs->asfile = NPP_StreamAsFile;
    pluginFuncs->writeready = NPP_WriteReady;
    pluginFuncs->write = NPP_Write;
    pluginFuncs->print = NPP_Print;
    pluginFuncs->event = NPP_HandleEvent;
    pluginFuncs->urlnotify = NPP_URLNotify;
    pluginFuncs->getvalue = NPP_GetValue;
    pluginFuncs->setvalue = NPP_SetValue;

    return NPERR_NO_ERROR;
  }

#if defined(OS_WINDOWS) || defined(OS_MACOSX)
  NPError OSCALL NP_Initialize(NPNetscapeFuncs *browserFuncs) {
    return InitializeNPNApi(browserFuncs);
  }
#else
  NPError OSCALL NP_Initialize(NPNetscapeFuncs *browserFuncs,
                               NPPluginFuncs *pluginFuncs) {
    NPError retval = InitializeNPNApi(browserFuncs);
    if (retval != NPERR_NO_ERROR) return retval;
    NP_GetEntryPoints(pluginFuncs);
    return NPERR_NO_ERROR;
  }
#endif

  NPError OSCALL NP_Shutdown(void) {
    return NPERR_NO_ERROR;
  }

  NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode,
                  int16_t argc, char *argn[], char *argv[],
                  NPSavedData *saved) {
    glue::InitializeGlue(instance);
    NPObject *object = glue::CreateStaticNPObject(instance);
    instance->pdata = object;

#if defined(OS_MACOSX)
    // On the Mac, starting from Google Chrome 22, we need to negotiate
    // CoreGraphics and Cocoa even though we don't use it.
    // See: http://code.google.com/p/chromium/issues/detail?id=134761
    // Code ported from http://gwt-code-reviews.appspot.com/1844803/

    // Select the right drawing model if necessary
    NPBool supports_core_graphics = false;
    if (NPN_GetValue(instance, NPNVsupportsCoreGraphicsBool,
                     &supports_core_graphics) == NPERR_NO_ERROR &&
        supports_core_graphics) {
      NPN_SetValue(instance, NPPVpluginDrawingModel,
                   reinterpret_cast<void*>(NPDrawingModelCoreGraphics));
    }
    // Select the Cocoa event model
    NPBool supports_cocoa_events = false;
    if (NPN_GetValue(instance, NPNVsupportsCocoaBool,
                     &supports_cocoa_events) == NPERR_NO_ERROR &&
        supports_cocoa_events) {
      NPN_SetValue(instance, NPPVpluginEventModel,
                   reinterpret_cast<void*>(NPEventModelCocoa));
    }
#endif
    return NPERR_NO_ERROR;
  }

  NPError NPP_Destroy(NPP instance, NPSavedData **save) {
    NPObject *object = static_cast<NPObject*>(instance->pdata);
    if (object) {
      NPN_ReleaseObject(object);
      instance->pdata = NULL;
    }
    return NPERR_NO_ERROR;
  }

  NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
    switch (variable) {
      case NPPVpluginScriptableNPObject: {
        void **v = static_cast<void **>(value);
        NPObject *obj = static_cast<NPObject *>(instance->pdata);
        NPN_RetainObject(obj);
        *v = obj;
        break;
      }
      case NPPVpluginNeedsXEmbed:
        *static_cast<NPBool *>(value) = true;
        break;
      default:
        return NPERR_INVALID_PARAM;
        break;
    }
    return NPERR_NO_ERROR;
  }

  NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value) {
    return NPERR_GENERIC_ERROR;
  }


  NPError NPP_SetWindow(NPP instance, NPWindow *window) {
    return NPERR_NO_ERROR;
  }

  void NPP_StreamAsFile(NPP instance, NPStream *stream, const char *fname) {
  }

  int16_t NPP_HandleEvent(NPP instance, void *event) {
    return 0;
  }

  NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream *stream,
                        NPBool seekable, uint16_t *stype) {
    return NPERR_NO_ERROR;
  }

  NPError NPP_DestroyStream(NPP instance, NPStream *stream, NPReason reason) {
    return NPERR_NO_ERROR;
  }

  int32_t NPP_WriteReady(NPP instance, NPStream *stream) {
    return 0;
  }

  int32_t NPP_Write(NPP instance, NPStream *stream, int32_t offset, int32_t len,
                    void *buffer) {
    return 0;
  }

  void NPP_Print(NPP instance, NPPrint *platformPrint) {
  }

  void NPP_URLNotify(NPP instance, const char *url, NPReason reason,
                     void *notifyData) {
  }
}  // extern "C"
