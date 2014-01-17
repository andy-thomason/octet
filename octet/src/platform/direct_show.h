
#define _WS2DEF_
#define _WINSOCK2API_
#pragma warning(disable : 4995)
#include <dshow.h>
#pragma comment( lib, "strmiids.lib" )

// avoid DirectX dependencies by putting this interface inline
struct ISampleGrabberCB;

// in strmiids.lib
EXTERN_C const CLSID CLSID_SampleGrabber;
EXTERN_C const IID IID_ISampleGrabber;

//MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
struct ISampleGrabber : public IUnknown
{
  virtual HRESULT STDMETHODCALLTYPE SetOneShot( BOOL OneShot) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetMediaType( const AM_MEDIA_TYPE *pType) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( AM_MEDIA_TYPE *pType) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( BOOL BufferThem) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( /* [out][in] */ long *pBufferSize, /* [out] */ long *pBuffer) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( /* [retval][out] */ IMediaSample **ppSample) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetCallback( ISampleGrabberCB *pCallback, long WhichMethodToCallback) = 0;
};

namespace octet {
  class video_capture {
    // ultra-irritating and overly complex capture graph mechanism. Thanks Microsoft...
    struct state {
      IGraphBuilder *Graph;
      ICaptureGraphBuilder2 *Builder;
      IBaseFilter *Src;
      ISampleGrabber *Grab;
      IMediaControl *Control;
      VIDEOINFOHEADER info;
    } s;
    
    void check(HRESULT hr, const char *func, const char *err) {
      if (FAILED(hr)) {
        printf("failure in %s: %s\n", func, err);
        exit(1);
      }
    }

    int open_direct_show() {
      memset(&s, 0, sizeof(s));
      const char *func = "open_direct_show";

      check( CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&s.Graph), func, "no FilterGraph");
      check( CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&s.Builder), func, "no CaptureGraphBuilder2");
      check( s.Builder->SetFiltergraph(s.Graph), func, "SetFiltergraph");

      ICreateDevEnum *device_enumerator = NULL;
      check( CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&device_enumerator), func, "SystemDeviceEnum");

      // Create an enumerator for video capture devices.
      IEnumMoniker *class_enumerator = NULL;
      check( device_enumerator->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &class_enumerator, 0), func, "CreateClassEnumerator");
      device_enumerator->Release();

      // get the first device for now...
      ULONG num_fetched = 0;
      IMoniker *pMoniker = NULL;
      //for (int i = 0; i <= s.devnum; i++)
      {
        check( class_enumerator->Next(1, &pMoniker, &num_fetched), func, "Next");
      }
      class_enumerator->Release();

      // Bind the moniker to a filter object.
      if (s.Src == NULL)
      {
        check( pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&s.Src), func, "BindToObject");
      }
      pMoniker->Release();

      // Now add the capture filter to the graph
      check( s.Graph->AddFilter(s.Src, L"VideoCapture"), func, "AddFilter");

      IBaseFilter *filter = NULL;
      check( CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC, IID_IBaseFilter, (LPVOID *)&filter), func, "SampleGrabber");
      check( filter->QueryInterface(IID_ISampleGrabber, (void **)&s.Grab), func, "ISampleGrabber");
      check( s.Graph->AddFilter(filter, L"Grabber"), func, "AddFilter");

      check( s.Graph->QueryInterface(IID_IMediaControl, (LPVOID *)&s.Control), func, "IID_IMediaControl" );

      // set up the format
      AM_MEDIA_TYPE   mt;
      memset(&mt, 0, sizeof(AM_MEDIA_TYPE));
      mt.majortype = MEDIATYPE_Video;
      mt.subtype = MEDIASUBTYPE_RGB24;
      mt.formattype = FORMAT_VideoInfo;
      check( s.Grab->SetMediaType(&mt), func, "SetMediaType");


      // kick off the capture
      check( s.Builder->RenderStream( &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, s.Src, filter, NULL ), func, "RenderStream");

      filter->Release();
      //pNullRenderer->Release();
    
      s.Grab->SetBufferSamples(TRUE);
      s.Grab->SetOneShot(FALSE);

      AM_MEDIA_TYPE MediaType;
      check( s.Grab->GetConnectedMediaType(&MediaType), func, "GetConnectedMediaType");

      // Get a pointer to the video header.
      s.info = *(VIDEOINFOHEADER*)MediaType.pbFormat;

      // return the state, buffer, width, height, format
      //lua_newuserdata(lua, pVideoHeader->bmiHeader.biSizeImage);
      //lua_pushinteger(lua, pVideoHeader->bmiHeader.biWidth);
      //lua_pushinteger(lua, pVideoHeader->bmiHeader.biHeight);
      //lua_pushinteger(lua, pVideoHeader->bmiHeader.biBitCount == 24 ? GL_RGB : 0);

      CoTaskMemFree(MediaType.pbFormat);
      return 0;
    }

    int read_direct_show(void *buffer, unsigned max_size) {
      //printf("[\n"); fflush(stdout);
      const char *func = "read_direct_show";

      check( s.Control->Run(), func, "Run");

      long buffer_size = max_size;
      HRESULT hr = s.Grab->GetCurrentBuffer(&buffer_size, (long *)buffer);
      //printf("%08x]\n", hr); fflush(stdout);
      if (hr == VFW_E_WRONG_STATE)
      {
        // must retry
        return 1;
      } else if (hr != S_OK)
      {
        return -1;
      }
      //assert(buffer_size == image_size());

      return 0;
    }

    int close_direct_show() {
      const char *func = "close_direct_show";
      if (s.Graph) s.Graph->Release();
      if (s.Builder) s.Builder->Release();
      if (s.Src) s.Src->Release();
      if (s.Grab) s.Grab->Release();
      if (s.Control) s.Control->Release();
      return 0;
    }

    int direct_show_image_size() {
      long buffer_size = 0;
      s.Grab->GetCurrentBuffer(&buffer_size, NULL);
      return (int)buffer_size;
    }
  public:
    int open() {
      return open_direct_show();
    }

    int read(void *buffer, unsigned max_size) {
      return read_direct_show(buffer, max_size);
    }

    int close() {
      return close_direct_show();
    }

    unsigned width() { return s.info.bmiHeader.biWidth; }
    unsigned height() { return s.info.bmiHeader.biHeight; }
    unsigned bits_per_pixel() { return s.info.bmiHeader.biBitCount; }
    unsigned image_size() { return direct_show_image_size(); }
  };
}
