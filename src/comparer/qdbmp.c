#include "qdbmp.h"
#include <stdlib.h>
#include <string.h>


/* Bitmap header */
typedef struct _BMP_Header
{
	USHORT		Magic;				/* Magic identifier: "BM" */
	UINT		FileSize;			/* Size of the BMP file in bytes */
	USHORT		Reserved1;			/* Reserved */
	USHORT		Reserved2;			/* Reserved */
	UINT		DataOffset;			/* Offset of image data relative to the file's start */
	UINT		HeaderSize;			/* Size of the header in bytes */
	UINT		Width;				/* Bitmap's width */
	UINT		Height;				/* Bitmap's height */
	USHORT		Planes;				/* Number of color planes in the bitmap */
	USHORT		BitsPerPixel;		/* Number of bits per pixel */
	UINT		CompressionType;	/* Compression type */
	UINT		ImageDataSize;		/* Size of uncompressed image's data */
	UINT		HPixelsPerMeter;	/* Horizontal resolution (pixels per meter) */
	UINT		VPixelsPerMeter;	/* Vertical resolution (pixels per meter) */
	UINT		ColorsUsed;			/* Number of color indexes in the color table that are actually used by the bitmap */
	UINT		ColorsRequired;		/* Number of color indexes that are required for displaying the bitmap */
} BMP_Header;


/* Private data structure */
struct _BMP
{
	BMP_Header	Header;
	UCHAR*		Palette;
	UCHAR*		Data;
};


/* Holds the last error code */
static BMP_STATUS BMP_LAST_ERROR_CODE = 0;


/* Error description strings */
static const char* BMP_ERROR_STRING[] =
{
	"",
	"General error",
	"Could not allocate enough memory to complete the operation",
	"File input/output error",
	"File not found",
	"File is not a supported BMP variant (must be uncompressed 8, 24 or 32 BPP)",
	"File is not a valid BMP image",
	"An argument is invalid or out of range",
	"The requested action is not compatible with the BMP's type"
};


/* Size of the palette data for 8 BPP bitmaps */
#define BMP_PALETTE_SIZE	( 256 * 4 )



/*********************************** Forward declarations **********************************/
int		ReadHeader	( BMP* bmp, FILE* f );

int		ReadUINT	( UINT* x, FILE* f );
int		ReadUSHORT	( USHORT *x, FILE* f );






/*********************************** Public methods **********************************/


/**************************************************************
	Frees all the memory used by the specified BMP image.
**************************************************************/
void BMP_Free( BMP* bmp )
{
	if ( bmp == NULL )
	{
		return;
	}

	if ( bmp->Palette != NULL )
	{
		free( bmp->Palette );
	}

	if ( bmp->Data != NULL )
	{
		free( bmp->Data );
	}

	free( bmp );

	BMP_LAST_ERROR_CODE = BMP_OK;
}


/**************************************************************
	Reads the specified BMP image file.
**************************************************************/
BMP* BMP_ReadFile( const char* filename )
{
	BMP*	bmp;
	FILE*	f;

	if ( filename == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
		return NULL;
	}


	/* Allocate */
	bmp = calloc( 1, sizeof( BMP ) );
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_OUT_OF_MEMORY;
		return NULL;
	}


	/* Open file */
	f = fopen( filename, "rb" );
	if ( f == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_FILE_NOT_FOUND;
		free( bmp );
		return NULL;
	}


	/* Read header */
	if ( ReadHeader( bmp, f ) != BMP_OK || bmp->Header.Magic != 0x4D42 )
	{
		BMP_LAST_ERROR_CODE = BMP_FILE_INVALID;
		fclose( f );
		free( bmp );
		return NULL;
	}


	/* Verify that the bitmap variant is supported */
	if ( ( bmp->Header.BitsPerPixel != 32 && bmp->Header.BitsPerPixel != 24 && bmp->Header.BitsPerPixel != 8 )
		|| bmp->Header.CompressionType != 0 || bmp->Header.HeaderSize != 40 )
	{
		BMP_LAST_ERROR_CODE = BMP_FILE_NOT_SUPPORTED;
		fclose( f );
		free( bmp );
		return NULL;
	}


	/* Allocate and read palette */
	if ( bmp->Header.BitsPerPixel == 8 )
	{
		bmp->Palette = (UCHAR*) malloc( BMP_PALETTE_SIZE * sizeof( UCHAR ) );
		if ( bmp->Palette == NULL )
		{
			BMP_LAST_ERROR_CODE = BMP_OUT_OF_MEMORY;
			fclose( f );
			free( bmp );
			return NULL;
		}

		if ( fread( bmp->Palette, sizeof( UCHAR ), BMP_PALETTE_SIZE, f ) != BMP_PALETTE_SIZE )
		{
			BMP_LAST_ERROR_CODE = BMP_FILE_INVALID;
			fclose( f );
			free( bmp->Palette );
			free( bmp );
			return NULL;
		}
	}
	else	/* Not an indexed image */
	{
		bmp->Palette = NULL;
	}


	/* Allocate memory for image data */
	bmp->Data = (UCHAR*) malloc( bmp->Header.ImageDataSize );
	if ( bmp->Data == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_OUT_OF_MEMORY;
		fclose( f );
		free( bmp->Palette );
		free( bmp );
		return NULL;
	}


	/* Read image data */
	if ( fread( bmp->Data, sizeof( UCHAR ), bmp->Header.ImageDataSize, f ) != bmp->Header.ImageDataSize )
	{
		BMP_LAST_ERROR_CODE = BMP_FILE_INVALID;
		fclose( f );
		free( bmp->Data );
		free( bmp->Palette );
		free( bmp );
		return NULL;
	}


	fclose( f );

	BMP_LAST_ERROR_CODE = BMP_OK;

	return bmp;
}


/**************************************************************
	Returns the image's width.
**************************************************************/
UINT BMP_GetWidth( BMP* bmp )
{
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
		return -1;
	}

	BMP_LAST_ERROR_CODE = BMP_OK;

	return ( bmp->Header.Width );
}


/**************************************************************
	Returns the image's height.
**************************************************************/
UINT BMP_GetHeight( BMP* bmp )
{
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
		return -1;
	}

	BMP_LAST_ERROR_CODE = BMP_OK;

	return ( bmp->Header.Height );
}


/**************************************************************
	Returns the image's color depth (bits per pixel).
**************************************************************/
USHORT BMP_GetDepth( BMP* bmp )
{
	if ( bmp == NULL )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
		return -1;
	}

	BMP_LAST_ERROR_CODE = BMP_OK;

	return ( bmp->Header.BitsPerPixel );
}


/**************************************************************
	Populates the arguments with the specified pixel's RGB
	values.
**************************************************************/
void BMP_GetPixelRGB( BMP* bmp, UINT x, UINT y, UCHAR* r, UCHAR* g, UCHAR* b )
{
	UCHAR*	pixel;
	UINT	bytes_per_row;
	UCHAR	bytes_per_pixel;

	if ( bmp == NULL || x < 0 || x >= bmp->Header.Width || y < 0 || y >= bmp->Header.Height )
	{
		BMP_LAST_ERROR_CODE = BMP_INVALID_ARGUMENT;
	}
	else
	{
		BMP_LAST_ERROR_CODE = BMP_OK;

		bytes_per_pixel = bmp->Header.BitsPerPixel >> 3;

		/* Row's size is rounded up to the next multiple of 4 bytes */
		bytes_per_row = bmp->Header.ImageDataSize / bmp->Header.Height;

		/* Calculate the location of the relevant pixel (rows are flipped) */
		pixel = bmp->Data + ( ( bmp->Header.Height - y - 1 ) * bytes_per_row + x * bytes_per_pixel );


		/* In indexed color mode the pixel's value is an index within the palette */
		if ( bmp->Header.BitsPerPixel == 8 )
		{
			pixel = bmp->Palette + *pixel * 4;
		}

		/* Note: colors are stored in BGR order */
		if ( r )	*r = *( pixel + 2 );
		if ( g )	*g = *( pixel + 1 );
		if ( b )	*b = *( pixel + 0 );
	}
}


/**************************************************************
	Gets the specified pixel's color index.
**************************************************************/


/**************************************************************
	Returns the last error code.
**************************************************************/
BMP_STATUS BMP_GetError()
{
	return BMP_LAST_ERROR_CODE;
}


/**************************************************************
	Returns a description of the last error code.
**************************************************************/
const char* BMP_GetErrorDescription()
{
	if ( BMP_LAST_ERROR_CODE > 0 && BMP_LAST_ERROR_CODE < BMP_ERROR_NUM )
	{
		return BMP_ERROR_STRING[ BMP_LAST_ERROR_CODE ];
	}
	else
	{
		return NULL;
	}
}





/*********************************** Private methods **********************************/


/**************************************************************
	Reads the BMP file's header into the data structure.
	Returns BMP_OK on success.
**************************************************************/
int	ReadHeader( BMP* bmp, FILE* f )
{
	if ( bmp == NULL || f == NULL )
	{
		return BMP_INVALID_ARGUMENT;
	}

	/* The header's fields are read one by one, and converted from the format's
	little endian to the system's native representation. */
	if ( !ReadUSHORT( &( bmp->Header.Magic ), f ) )			return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.FileSize ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUSHORT( &( bmp->Header.Reserved1 ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUSHORT( &( bmp->Header.Reserved2 ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.DataOffset ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.HeaderSize ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.Width ), f ) )			return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.Height ), f ) )			return BMP_IO_ERROR;
	if ( !ReadUSHORT( &( bmp->Header.Planes ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUSHORT( &( bmp->Header.BitsPerPixel ), f ) )	return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.CompressionType ), f ) )	return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.ImageDataSize ), f ) )	return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.HPixelsPerMeter ), f ) )	return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.VPixelsPerMeter ), f ) )	return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.ColorsUsed ), f ) )		return BMP_IO_ERROR;
	if ( !ReadUINT( &( bmp->Header.ColorsRequired ), f ) )	return BMP_IO_ERROR;

	return BMP_OK;
}


/**************************************************************
	Reads a little-endian unsigned int from the file.
	Returns non-zero on success.
**************************************************************/
int	ReadUINT( UINT* x, FILE* f )
{
	UCHAR little[ 4 ];	/* BMPs use 32 bit ints */

	if ( x == NULL || f == NULL )
	{
		return 0;
	}

	if ( fread( little, 4, 1, f ) != 1 )
	{
		return 0;
	}

	*x = ( little[ 3 ] << 24 | little[ 2 ] << 16 | little[ 1 ] << 8 | little[ 0 ] );

	return 1;
}


/**************************************************************
	Reads a little-endian unsigned short int from the file.
	Returns non-zero on success.
**************************************************************/
int	ReadUSHORT( USHORT *x, FILE* f )
{
	UCHAR little[ 2 ];	/* BMPs use 16 bit shorts */

	if ( x == NULL || f == NULL )
	{
		return 0;
	}

	if ( fread( little, 2, 1, f ) != 1 )
	{
		return 0;
	}

	*x = ( little[ 1 ] << 8 | little[ 0 ] );

	return 1;
}


