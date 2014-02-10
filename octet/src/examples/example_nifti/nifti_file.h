////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// internal representation of a NIFTI file
  class nifti_file : public resource {
    /// shouty constants are no longer considered restful, but for compatibility
    /// we may have to persevere.
    enum {
      /// types
      NIFTI_TYPE_UINT8 = 2,
      NIFTI_TYPE_INT16 = 4,
      NIFTI_TYPE_INT32 = 8,
      NIFTI_TYPE_FLOAT32 = 16,
      NIFTI_TYPE_COMPLEX64 = 32,
      NIFTI_TYPE_FLOAT64 = 64,
      NIFTI_TYPE_RGB24 = 128,
      NIFTI_TYPE_INT8 = 256,
      NIFTI_TYPE_UINT16 = 512,
      NIFTI_TYPE_UINT32 = 768,
      NIFTI_TYPE_INT64 = 1024,
      NIFTI_TYPE_UINT64 = 1280,
      NIFTI_TYPE_FLOAT128 = 1536,
      NIFTI_TYPE_COMPLEX128 = 1792,
      NIFTI_TYPE_COMPLEX256 = 2048,
      NIFTI_TYPE_RGBA32 = 2304,

      /// intents
      NIFTI_INTENT_NONE = 0,
      NIFTI_INTENT_CORREL = 2,
      NIFTI_INTENT_TTEST = 3,
      NIFTI_INTENT_FTEST = 4,
      NIFTI_INTENT_ZSCORE = 5,
      NIFTI_INTENT_CHISQ = 6,
      NIFTI_INTENT_BETA = 7,
      NIFTI_INTENT_BINOM = 8,
      NIFTI_INTENT_GAMMA = 9,
      NIFTI_INTENT_POISSON = 10,
      NIFTI_INTENT_NORMAL = 11,
      NIFTI_INTENT_FTEST_NONC = 12,
      NIFTI_INTENT_CHISQ_NONC = 13,
      NIFTI_INTENT_LOGISTIC = 14,
      NIFTI_INTENT_LAPLACE = 15,
      NIFTI_INTENT_UNIFORM = 16,
      NIFTI_INTENT_TTEST_NONC = 17,
      NIFTI_INTENT_WEIBULL = 18,
      NIFTI_INTENT_CHI = 19,
      NIFTI_INTENT_INVGAUSS = 20,
      NIFTI_INTENT_EXTVAL = 21,
      NIFTI_INTENT_PVAL = 22,
      NIFTI_INTENT_LOGPVAL = 23,
      NIFTI_INTENT_LOG10PVAL = 24,
      NIFTI_FIRST_STATCODE = 2,
      NIFTI_LAST_STATCODE = 24,
      NIFTI_INTENT_ESTIMATE = 1001,
      NIFTI_INTENT_LABEL = 1002,
      NIFTI_INTENT_NEURONAME = 1003,
      NIFTI_INTENT_GENMATRIX = 1004,
      NIFTI_INTENT_SYMMATRIX = 1005,
      NIFTI_INTENT_DISPVECT = 1006,
      NIFTI_INTENT_VECTOR = 1007,
      NIFTI_INTENT_POINTSET = 1008,
      NIFTI_INTENT_TRIANGLE = 1009,
      NIFTI_INTENT_QUATERNION = 1010,
      NIFTI_INTENT_DIMLESS = 1011,
      NIFTI_INTENT_TIME_SERIES = 2001,
      NIFTI_INTENT_NODE_INDEX = 2002,
      NIFTI_INTENT_RGB_VECTOR = 2003,
      NIFTI_INTENT_RGBA_VECTOR = 2004,
      NIFTI_INTENT_SHAPE = 2005,
      NIFTI_XFORM_UNKNOWN = 0,
      NIFTI_XFORM_SCANNER_ANAT = 1,
      NIFTI_XFORM_ALIGNED_ANAT = 2,
      NIFTI_XFORM_TALAIRACH = 3,
      NIFTI_XFORM_MNI_152 = 4,
      NIFTI_UNITS_UNKNOWN = 0,
      NIFTI_UNITS_METER = 1,
      NIFTI_UNITS_MM = 2,
      NIFTI_UNITS_MICRON = 3,
      NIFTI_UNITS_SEC = 8,
      NIFTI_UNITS_MSEC = 16,
      NIFTI_UNITS_USEC = 24,
      NIFTI_UNITS_HZ = 32,
      NIFTI_UNITS_PPM = 40,
      NIFTI_UNITS_RADS = 48,
      NIFTI_SLICE_UNKNOWN = 0,
      NIFTI_SLICE_SEQ_INC = 1,
      NIFTI_SLICE_SEQ_DEC = 2,
      NIFTI_SLICE_ALT_INC = 3,
      NIFTI_SLICE_ALT_DEC = 4,
      NIFTI_SLICE_ALT_INC2 = 5,
      NIFTI_SLICE_ALT_DEC2 = 6,
    };

    /// storage for the file.
    dynarray<uint8_t> buffer;

    /// zip decoder
    zip_decoder dec;

    struct nifti_header {
      int      sizeof_hdr;    /// MUST be 348
      char     data_type[10]; /// ++UNUSED++
      char     db_name[18];   /// ++UNUSED++
      int      extents;       /// ++UNUSED++
      short    session_error; /// ++UNUSED++
      char     regular;       /// ++UNUSED++
      char     dim_info;      /// MRI slice ordering.

                                          ///--- was image_dimension substruct ---
      short    dim[8];        /// Data array dimensions.
      float    intent_p1 ;    /// 1st intent parameter.
                                                          ///  short unused9;
      float    intent_p2 ;    /// 2nd intent parameter.
                                                          ///  short unused11;
      float    intent_p3 ;    /// 3rd intent parameter.
                                                          ///  short unused13;
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

    /// byte-swapped copy of header
    nifti_header h;

    void swap_header() {
      assert(0 && "todo: support byte ordering");
    }

    /// return true if the header is broken.
    // todo: if this becomes a more than a sketch, check the header thoroughly.
    bool check_header(const char *url) {
      if (buffer.size() < sizeof(nifti_header)) {
        log("bad or non-existant nifiti file %s\n", url);
        return true;
      }

      h = (nifti_header&)buffer[0];
      if (h.dim[0] >= 7) swap_header();

      if (
        h.sizeof_hdr != sizeof(nifti_header) ||
        (!memcmp(h.magic, "ni1", 4) && !memcmp(h.magic, "n+1", 4))
      ) {
        log("bad header nifiti file %s\n", url);
        return true;
      }

      const uint8_t *extension = &buffer[sizeof(nifti_header)];
      int vox_offset = (int)(float)h.vox_offset;
      int bitpix = h.bitpix;

      log("vox_offset = %08x\n", vox_offset);
      log("extension = %02x %02x %02x %02x\n", extension[0], extension[1], extension[2], extension[3]);

      int num_dim = h.dim[0];
      int tot_voxels = 1;
      for (int i = 1; i <= num_dim; ++i) {
        log("dim[%d] = %d   pd=%f\n", i, (int)h.dim[i], (float)h.pixdim[i]);
        tot_voxels *= h.dim[i];
      }

      log("bitpix=%d\n", bitpix);
      log("intent_code=%d\n", h.intent_code);
      log("expected size=%d\n", vox_offset + tot_voxels * bitpix / 8);
      log("actual size=%d\n", buffer.size());
      log("xyzt_units=%08x\n", h.xyzt_units);

      /*switch (h.intent_code) {
      }*/
      return false;
    }
  public:
    nifti_file() {
      assert(sizeof(nifti_header) == 348);
    }

    nifti_file(const char *url) {
      app_utils::get_url(buffer, url);
      if (check_header(url)) return;


    }
  };
}
