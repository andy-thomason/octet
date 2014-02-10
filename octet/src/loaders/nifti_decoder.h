////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//
namespace octet { namespace loaders {
  /// NIFTI NMR data decoder. ie. 3d textures.
  /// This loader only handles very simple NIFTI files.
  class nifti_decoder {
    unsigned vox_offset;
    unsigned layer_stride;
    unsigned frame_stride;

    struct nifti_header {
      int      sizeof_hdr;    /// MUST be 348
      char     data_type[10]; /// ++UNUSED++
      char     db_name[18];   /// ++UNUSED++
      int      extents;       /// ++UNUSED++
      short    session_error; /// ++UNUSED++
      char     regular;       /// ++UNUSED++
      char     dim_info;      /// MRI slice ordering.

      short    dim[8];        /// Data array dimensions.
      float    intent_p1 ;    /// 1st intent parameter.
      float    intent_p2 ;    /// 2nd intent parameter.
      float    intent_p3 ;    /// 3rd intent parameter.
      short    intent_code ;  /// NIFTI_INTENT_* code.
      short    datatype;      /// Defines data type!
      short    bitpix;        /// Number bits/voxel.
      short    slice_start;   /// First slice index.
      float    pixdim[8];     /// Grid spacings.
      float    vox_offset;    /// Offset into .nii file
      float    scl_slope ;    /// Data scaling: slope.
      float    scl_inter ;    /// Data scaling: offset.
      short    slice_end;     /// Last slice index.
      char     slice_code ;   /// Slice timing order.
      char     xyzt_units ;   /// Units of pixdim[1..4]
      float    cal_max;       /// Max display intensity
      float    cal_min;       /// Min display intensity
      float    slice_duration;/// Time for 1 slice.
      float    toffset;       /// Time axis shift.
      int      glmax;         /// ++UNUSED++
      int      glmin;         /// ++UNUSED++

                                              /// --- was data_history substruct ---
      char     descrip[80];   /// any text you like.
      char     aux_file[24];  /// auxiliary filename.

      short    qform_code ;   /// NIFTI_XFORM_* code.
      short    sform_code ;   /// NIFTI_XFORM_* code.
                                                          ///    are replaced
      float    quatern_b ;    /// Quaternion b param.
      float    quatern_c ;    /// Quaternion c param.
      float    quatern_d ;    /// Quaternion d param.
      float    qoffset_x ;    /// Quaternion x shift.
      float    qoffset_y ;    /// Quaternion y shift.
      float    qoffset_z ;    /// Quaternion z shift.

      float    srow_x[4] ;    /// 1st row affine transform.
      float    srow_y[4] ;    /// 2nd row affine transform.
      float    srow_z[4] ;    /// 3rd row affine transform.

      char    intent_name[16];/// 'name' or meaning of data.

      char    magic[4] ;      /// MUST be "ni1\0" or "n+1\0".
    };

  public:
    /// get data for a texture in memory.
    void get_image(dynarray<uint8_t> &bytes, uint16_t &format, uint16_t &width, uint16_t &height, uint16_t &depth, uint32_t &frames, const uint8_t *src, const uint8_t *src_max) {
      // convert the data
      nifti_header header = *(nifti_header*)src;
    
      width = 0;
      height = 0;
      format = 0;

      if (header.dim[0] != 4) {
        log("warning: NIFTI image type not supported (dim[0] = %d)\n", header.dim[0]);
        return;
      }

      width = header.dim[1];
      height = header.dim[2];
      depth = header.dim[3];
      frames = 1; //header.dim[4];
      vox_offset = (int)header.vox_offset;
      layer_stride = width * height * header.bitpix / 8;
      frame_stride = layer_stride * depth;
      unsigned size = frame_stride * frames;

      if ((int)header.vox_offset + (int)size > src_max - src) {
        log("warning: NIFTI image too small\n");
        return;
      }

      format = header.bitpix == 24 ? 0x1907 : 0x1908; // GL_RGB / GL_RGBA

      // get one frame (of 3D data)
      bytes.resize(frame_stride);
      memcpy(&bytes[0], src + vox_offset, frame_stride);
    }

    /// get the offset of a specific layer in a specific frame.
    unsigned get_layer_offset(unsigned layer, unsigned frame) {
      return vox_offset + layer * layer_stride + frame * frame_stride;
    }
  };
}}
