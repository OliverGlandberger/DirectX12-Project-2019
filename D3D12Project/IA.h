struct PIPELINEINPUT {
	struct IA {
		struct ROOTINDEX {
			enum {
				VS_CB_TRANSLATION = 0,
				VS_CB_VIEWPROJ = 1,
				PS_CB_DIFFUSE_TINT = 2,
				PS_TEXTURE = 3,
				COUNT = 4
			};
		};
		struct REGISTERINDEX {
			enum {
				POSITION = 0,
				NORMAL = 1,
				TEXCOORD = 2
			};
		};
	};
	struct CB {
		enum {
			TRANSLATION_MATRIX = 0,
			VIEWPROJ_MATRIX = 1,
			DIFFUSE_TINT = 2,
			COUNT = 3
		};
	};
};

// Left so that .HLSL files can be constructed in 'initializeTestBench' still work, however, for using D3D12
// shit, always use the structs/enums above.
#define POSITION 0
#define NORMAL 1
#define TEXTCOORD 2
#define INDEXBUFF 4

#define TRANSLATION_NAME "TranslationBlock"

#define VIEWPROJ_NAME "ViewProj"

#define DIFFUSE_TINT_NAME "DiffuseColor"

#define DIFFUSE_SLOT 0