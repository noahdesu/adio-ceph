#include "config.h"

#include "rfg_filter.h"

#include "vt_inttypes.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_LEN 0x20000 /* max file line length */

/* data structure for filter assignments */

typedef struct RFG_FilterAssigns_struct
{
  int32_t  climit;               /* call limit */
  uint32_t sbounds[2];           /* stack level bounds */
  uint8_t  flags;                /* flags bitmask (group, recursiveness) */
  char*    pattern;              /* pattern */
} RFG_FilterAssigns;

struct RFG_Filter_struct
{
  char*   deffile_name;         /* name of filter definition file */
  char*   deffile_content;      /* content of filter file */
  size_t  deffile_content_size; /* buffer size of filter file content */

  int32_t default_call_limit;   /* default call limit */

  uint32_t           nassigns;  /* number of filter assignments */
  RFG_FilterAssigns* assigns;   /* array of filter assignments */
};

static int get_deffile_content( RFG_Filter* filter )
{
  FILE* f;
  size_t i;
  uint8_t err = 0;
  const size_t bsize = 1024;

  if( !filter ) return 0;

  if( !filter->deffile_name ) return 0;

  /* open filter definition file */

  f = fopen( filter->deffile_name, "r" );
  if( !f ) return 0;

  filter->deffile_content = (char*)malloc( bsize * sizeof( char ) );
  if( filter->deffile_content == NULL ) err = 1;
  else filter->deffile_content_size = bsize;

  i = 0;
  while( !err &&
         ( ( filter->deffile_content[i++] = fgetc( f ) ) != (char)EOF ) )
  {
    /* enlarge buffer, if necessary */

    if( i == filter->deffile_content_size )
    {
      filter->deffile_content =
        (char*)realloc( filter->deffile_content,
                        ( filter->deffile_content_size + bsize ) *
                          sizeof( char ) );
      if( filter->deffile_content == NULL )
      {
        err = 1;
        break;
      }
      filter->deffile_content_size += bsize;
    }
  }

  /* append '\0' to buffer */
  if( !err) filter->deffile_content[i-1] = '\0';

  /* close filter definition file */
  fclose( f );

  return (int)!err;
}

static int get_deffile_content_line( RFG_Filter* filter, char* buf,
                                     size_t bufsize, size_t* pos )
{
  size_t content_size;
  size_t i;

  if( !filter ) return 0;

  if( !filter->deffile_content ) return 0;

  content_size = strlen( filter->deffile_content );

  if( *pos >= content_size ) return 0;

  for( i = 0; i < bufsize && *pos < content_size; i++ )
  {
    buf[i] = filter->deffile_content[(*pos)++];
    if( buf[i] == '\n' )
    {
      buf[i+1] = '\0';
      break;
    }
  }

  return 1;
}

RFG_Filter* RFG_Filter_init()
{
  RFG_Filter* ret;

  /* allocate memory for RFG filter object */

  ret = ( RFG_Filter* )malloc( sizeof( RFG_Filter ) );
  if( ret == NULL )
    return NULL;

  /* some initializes of data structure elements */

  ret->deffile_name = NULL;
  ret->deffile_content = NULL;
  ret->deffile_content_size = 0;
  ret->default_call_limit = -1;
  ret->nassigns = 0;
  ret->assigns = NULL;

  return ret;
}

int RFG_Filter_free( RFG_Filter* filter )
{
  if( !filter ) return 0;

  /* reset filter assignments */
  if( !RFG_Filter_reset( filter ) ) return 0;

  /* free filter definition file name */

  if( filter->deffile_name )
    free( filter->deffile_name );

  /* free filter file content buffer */

  if( filter->deffile_content )
    free( filter->deffile_content );

  /* free self */

  free( filter );
  filter = NULL;

  return 1;
}

int RFG_Filter_reset( RFG_Filter* filter )
{
  if( !filter ) return 0;

  if( filter->nassigns > 0 )
  {
    uint32_t i;

    /* free array of call limit assignments */

    for( i = 0; i < filter->nassigns; i++ )
      free( filter->assigns[i].pattern );
    free( filter->assigns );

    filter->assigns = NULL;
    filter->nassigns = 0;
  }

  return 1;
}

int RFG_Filter_setDefFile( RFG_Filter* filter, const char* deffile )
{
  if( !filter ) return 0;

  /* if a filter definition file already set, then free this */

  if( filter->deffile_name )
    free( filter->deffile_name );

  /* set new filter definition file */

  filter->deffile_name = strdup( deffile );

  return 1;
}

int RFG_Filter_setDefaultCallLimit( RFG_Filter* filter, int32_t limit )
{
  if( !filter ) return 0;

  if( limit == 0 || limit < -1 )
  {
    fprintf( stderr,
	     "RFG_Filter_setDefaultCallLimit(): Error: Default call limit must be greater than 0 or -1\n" );
    return 0;
  }

  /* set new default call limit */

  filter->default_call_limit = limit == -1 ? limit : limit+1;

  return 1;
}

int RFG_Filter_readDefFile( RFG_Filter* filter, int rank, uint8_t* rank_off )
{
  char*    line;
  uint32_t lineno = 0;
  size_t   pos = 0;
  uint8_t  parse_err = 0;
  uint8_t  l_rank_off = 0;
  uint8_t  includes_current_rank = 1;

  if( !filter ) return 0;

  if( !filter->deffile_name ) return 0;

  /* reset filter assignments */
  if( !RFG_Filter_reset( filter ) ) return 0;

  /* get filter file content, if necessary */

  if( !filter->deffile_content )
  {
    if( !get_deffile_content( filter ) )
    {
      fprintf( stderr,
               "RFG_Filter_readDefFile(): Error: Could not read file '%s'\n",
               filter->deffile_name );
      return 0;
    }
  }

  /* allocate memory for line */

  line = ( char* )malloc( MAX_LINE_LEN * sizeof( char ) );
  if( line == NULL ) return 0;

  /* read lines */
  while( !l_rank_off && !parse_err &&
         get_deffile_content_line( filter, line, MAX_LINE_LEN, &pos ) )
  {
    char* p;
    char* q;

    /* increment line number */
    lineno++;

    /* remove newline */
    if( strlen(line) > 0 && line[strlen(line)-1] == '\n' )
      line[strlen(line)-1] = '\0';

    /* remove leading and trailing spaces from line */
    vt_strtrim( line );

    /* cut possible comment from line */

    p = strchr( line, '#' );
    if( p != NULL )
      *p = '\0';

    /* continue if line is empty */
    if( strlen( line ) == 0 )
      continue;

    if( line[0] == '@' )
    {
      int a = -1;
      int b = -1;
      uint8_t is_rank_off_rule = 0;

      /* check whether selected ranks shall be disabled */
      if( ( p = strstr( line, "--" ) ) )
      {
        /* cut "-- OFF" from line */
        *p = '\0';

        p += 2;
        while( *p == ' ' || *p == '\t' ) p++;
        q = p;
        while( *q != '\0' ) { *q = tolower( *q ); q++; }

        if( strcmp( p, "off" ) == 0 )
        {
          is_rank_off_rule = 1;
        }
        else
        {
          parse_err = 1;
          break;
        }
      }

      /* no specific rank given? */
      if( rank == -1 )
      {
        /* continue reading, if selected ranks shall be disabled
           (non-rank-specific filter rules can follow) */
        if( is_rank_off_rule )
          continue;
        /* otherwise, stop reading
           (all the following filter rules are rank-specific which will be
            read later) */
        else
          break;
      }

      /* parse rank selection
         If current rank is included, then read the following filter rules.
         Otherwise, jump to next @ clause. */

      p = line + 1;

      includes_current_rank = 0;

      while( 1 )
      {
        while( *p == ' ' || *p == '\t' ) p++;

        if( *p >= '0' && *p <= '9' )
        {
          errno = 0;
          a = strtol( p, &q, 10 );
          p = q;
          if( errno != 0 )
          {
            parse_err = 1;
            break;
          }
        }
        else if( *p == '-' && *(p+1) != '\0' && a != -1 )
        {
          p++;

          errno = 0;
          b = strtol( p, &q, 10 );
          p = q;
          if( errno != 0 )
          {
            parse_err = 1;
            break;
          }
        }
        else if( (*p == ';' || *p == ',') && a != -1 )
        {
          p++;

          if( a == rank || (a < rank && rank <= b ) )
          {
            includes_current_rank = 1;
            if( is_rank_off_rule )
            {
              l_rank_off = 1; /* deactivate this rank completely */
              break;
            }
          }
          else if( is_rank_off_rule )
          {
            includes_current_rank = 1;
          }

          a = b = -1;
        }
        else if( *p == '\0' && a != -1 )
        {
          if( a == rank || (a < rank && rank <= b ) )
          {
            includes_current_rank = 1;
            if( is_rank_off_rule )
              l_rank_off = 1; /* deactivate this rank completely */
          }
          else if( is_rank_off_rule )
          {
            includes_current_rank = 1;
          }

          break;
        }
        else
        {
          parse_err = 1;
          break;
        }
      }
    }
    else
    {
      int32_t  climit;
      uint32_t sbounds[2] = { 1, (uint32_t)-1 };
      uint8_t  flags = 0;

      /* search for '--'
         e.g. "func1;func2;func3 -- 1000 S:5-10 R"
                                 p
      */

      p = strstr( line, "--" );
      if( p == NULL )
      {
        parse_err = 1;
        break;
      }

      /* cut call limit, stack level bounds, and flags from line
         e.g.   "func1;func2;func3 -- 1000 S:5-10 R"
             => "func1;func2;func3"
      */
      *p = '\0';

      /* split remaining line at ' ' to get call limit, stack level bounds,
         and flags */

      p = strtok( p+2, " " );
      if( p == NULL )
      {
        parse_err = 1;
        break;
      }

      /* parse call limit */
      {
        long l_climit;

        l_climit = strtol( p, &q, 10 );
        if( p == q || l_climit == LONG_MIN || l_climit == LONG_MAX ||
            l_climit < -1 )
        {
          parse_err = 1;
          break;
        }

        climit = (int32_t)l_climit;
      }

      /* parse stack level bounds and flags */
      while( !parse_err && ( p = strtok( NULL, " " ) ) )
      {
        /* stack level bounds */
        if( strlen( p ) > 2 && tolower( *p ) == 's' && *(p+1) == ':' )
        {
          long l_bounds[2] = { 1 };

          p = p+2;

          /* parse maximum (or minimum) stack level bound */

          l_bounds[1] = strtol( p, &q, 10 );
          if( p == q || l_bounds[1] == LONG_MIN || l_bounds[1] == LONG_MAX ||
              l_bounds[1] < 1 )
          {
            parse_err = 1;
            break;
          }

          p = q;

          /* minimum stack level bound specified? */
          if( *p == '-' )
          {
            p++;
            l_bounds[0] = l_bounds[1];

            /* parse maximum stack level bound */

            l_bounds[1] = strtol( p, &q, 10 );
            if( p == q || l_bounds[1] == LONG_MIN || l_bounds[1] == LONG_MAX ||
                l_bounds[1] < l_bounds[0] )
            {
              parse_err = 1;
              break;
            }

            p = q;
          }

          if( strlen( p ) > 1 || ( *p != '\0' && *p != '\t' ) )
          {
            parse_err = 1;
            break;
          }

          sbounds[0] = (uint32_t)l_bounds[0];
          sbounds[1] = (uint32_t)l_bounds[1];
        }
        /* group flag */
        else if( strlen( p ) == 1 && tolower( *p ) == 'g' )
        {
          flags |= RFG_FILTER_FLAG_GROUP;
        }
        /* recursiveness flag */
        else if( strlen( p ) == 1 && tolower( *p ) == 'r' )
        {
          flags |= RFG_FILTER_FLAG_RECURSIVE;
        }
        else
        {
          parse_err = 1;
          break;
        }
      }
      if( parse_err )
        break;

      /* split line at ';' to get pattern */

      p = strtok( line, ";" );
      do
      {
        char* pattern;

        if( !p )
        {
          parse_err = 1;
          break;
        }

        pattern = strdup( p );
        vt_strtrim( pattern );

        /* add call limit assignment */
        if( strlen( pattern ) > 0 && includes_current_rank )
          RFG_Filter_add( filter, pattern, climit, sbounds, flags );

        free( pattern );

      } while( ( p = strtok( 0, ";" ) ) );
    }
  }

  free( line );

  if( parse_err )
  {
    fprintf( stderr, "%s:%u: Could not be parsed\n",
             filter->deffile_name, lineno );
    return 0;
  }

  if( rank_off )
    *rank_off = l_rank_off;

  return 1;
}

int RFG_Filter_add( RFG_Filter* filter, const char* pattern, int32_t climit,
                    uint32_t* sbounds, uint8_t flags )
{
  if( !filter || !pattern ) return 0;

  /* enlarge array of filter assignments */

  filter->assigns =
    (RFG_FilterAssigns*)realloc( filter->assigns,
                                 ( filter->nassigns + 1 )
                                 * sizeof( RFG_FilterAssigns ) );

  if( filter->assigns == NULL )
    return 0;

  /* add new filter assignment */

  filter->assigns[filter->nassigns].climit = climit;
  filter->assigns[filter->nassigns].sbounds[0] = 1;
  filter->assigns[filter->nassigns].sbounds[1] = (uint32_t)-1;
  if( sbounds )
  {
     filter->assigns[filter->nassigns].sbounds[0] = sbounds[0];
     filter->assigns[filter->nassigns].sbounds[1] = sbounds[1];
  }
  filter->assigns[filter->nassigns].flags = flags;
  filter->assigns[filter->nassigns].pattern = strdup( pattern );
  filter->nassigns++;

  return 1;
}

int RFG_Filter_get( RFG_Filter* filter, const char* rname, const char* gname,
                    int32_t* r_climit, uint32_t* r_sbounds, uint8_t* r_flags )
{
  uint32_t i;

  if( !filter || ( !rname && !gname ) )
    return 0;

  /* initialize return parameters by defaults */
  if( r_climit != NULL )
    *r_climit = filter->default_call_limit;
  if( r_sbounds != NULL )
  {
    r_sbounds[0] = 1;
    r_sbounds[1] = (uint32_t)-1;
  }
  if( r_flags != NULL )
    *r_flags = 0;

  /* search for matching filter rule either by ... */
  for( i = 0; i < filter->nassigns; i++ )
  {
    const uint8_t is_group_rule =
      (filter->assigns[i].flags & RFG_FILTER_FLAG_GROUP) != 0;

    if( /* ... region group name */
        ( is_group_rule && gname &&
          fnmatch( filter->assigns[i].pattern, gname, 0 ) == 0 ) ||
        /* ... or by region name */
        ( !is_group_rule && rname &&
          fnmatch( filter->assigns[i].pattern, rname, 0 ) == 0 ) )
    {
      /* set return parameters regarding to found filter rules */
      if( r_climit != NULL )
        *r_climit = filter->assigns[i].climit;
      if( r_sbounds != NULL )
      {
        r_sbounds[0] = filter->assigns[i].sbounds[0];
        r_sbounds[1] = filter->assigns[i].sbounds[1];
      }
      if( r_flags != NULL )
        *r_flags = filter->assigns[i].flags;

      /* abort searching on first matching filter rule */
      break;
    }
  }

  return 1;
}
