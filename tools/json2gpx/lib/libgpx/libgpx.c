#include <libgpx.h>

#define LIBGPX__LINK_OPEN "<link"
#define GPX_READER__TRK_OPEN "<trk>"
#define GPX_READER__TRKPT_OPEN "<trkpt"
#define GPX_READER__WYPT_OPEN "<wpt"
#define GPX_READER__TRKSEG_OPEN "<trkseg>"
#define GPX_READER__RTEPT_OPEN "<rtept"

#define LIBGPX__MAXIMUM_XML_DEPTH 10 /**< Defense against billion laughs etc... */

static char *libgpx__strtok(const char *src, const char *str) {
  char *t = (char *)&src[0];
  char *s = (char *)&str[0];
  char *fo;
  while(*t != '\0'){
    if(*t == *s){fo = t;}
    while(*t == *s){
      ++t;
      ++s;
      if(*t == '\0'){return NULL;}
      if(*s == '\0'){return fo;}
    }
    s = (char *)&str[0];
    ++t;
  }
  return NULL;
}

static int libgpx__strlen(const char* str){
  int i = 0;
  char *s = (char *)&str[0];
  while(*s != '\0'){++i; ++s;}
  return i;
}

static void libgpx__strcpy(char *dest, const char *s, int len){
  int i = 0;
  while(i < len && *s!='\0'){
    *dest = *s;
    ++dest;
    ++s;
    ++i;
  }
  *dest = '\0';
}

static int libgpx__strtoi(const char *s){
  int i = 0;
  int f = 1;
  while(*s!='\0'){
    i = i * 10;
    if(*s == '-'){
      f = -1;
    } else {
      i = i + *s - '0';
    }
    ++s;
  }
  return i * f;
}

static double libgpx__strtod(const char* s){
  double rez = 0, fact = 1;
  if (*s == '-'){
    ++s;
    fact = -1;
  };
  int point_seen = 0;
  while (*s){
    if (*s == '.'){
      point_seen = 1;
      ++s;
      continue;
    };
    int d = *s - '0';
    if (d >= 0 && d <= 9){
      if (point_seen) fact /= 10.0f;
      rez = rez * 10.0f + (double)d;
    };
    ++s;
  };
  return rez * fact;
};

unsigned int count_needles_in_haystack(const char *haystack, const char *needle){
  unsigned int count = 0;
    while ( (haystack=libgpx__strtok(haystack, needle)) != NULL ) {
        haystack += libgpx__strlen(needle);
        if(count < UINT_MAX){
          ++count;
        }
    }
    if(count == INT_MAX){
      return 0;
    }
    return count;
}

int libgpx_count_gpx_types(char *gpx, libgpx_GPX *sgpx){
  unsigned int link_count = 0;
  unsigned int point_count = 0;
  unsigned int route_point_count;
  unsigned int track_point_count;
  unsigned int waypoint_count;
  unsigned int group_count = 0;
  track_point_count = count_needles_in_haystack(gpx, GPX_READER__TRKPT_OPEN);
  route_point_count = count_needles_in_haystack(gpx, GPX_READER__RTEPT_OPEN);
  waypoint_count = count_needles_in_haystack(gpx, GPX_READER__WYPT_OPEN);
  
  if(track_point_count > INT_MAX - route_point_count - waypoint_count - 1){
    return -1;
  }
  else{
    point_count = track_point_count + route_point_count + waypoint_count + 1;
  }

  link_count += count_needles_in_haystack(gpx, LIBGPX__LINK_OPEN);
  if(link_count < INT_MAX - 1){
    if(link_count > 0){
      ++link_count;
    } else{
      link_count = 1;
    }
  } else {
    return -1;
  }

  group_count += count_needles_in_haystack(gpx, GPX_READER__TRK_OPEN);
  group_count += count_needles_in_haystack(gpx, GPX_READER__TRKSEG_OPEN);
  sgpx->n_GPXPoint = point_count;
  sgpx->n_GPXLink = link_count;
  sgpx->n_GPXPointCollection = group_count + 1;
  return 1;
}

static void libgpx_parse_link(xmlNode *node, libgpx_GPX *sgpx, int member_of, int *xmlDepth, size_t *link){
  ++(*xmlDepth);
  if(*xmlDepth > LIBGPX__MAXIMUM_XML_DEPTH){
    --(*xmlDepth);
    return;
  }

  sgpx->links[*link].member_of = member_of;
  char *s = &sgpx->links[*link].href[0];
  *s = '\0';
  s = &sgpx->links[*link].type[0];
  *s = '\0';
  s = &sgpx->links[*link].text[0];
  *s = '\0';

  if(xmlGetProp(node, (const xmlChar *)"href")){
    libgpx__strcpy(sgpx->links[*link].href, (const char *)xmlGetProp(node, (const xmlChar *)"href"), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
  }
  
  xmlNode *deeperNode;
  deeperNode = node->children;
  while(deeperNode){
    if(!xmlStrcmp(deeperNode->name, (const xmlChar *)"text")){
      libgpx__strcpy(sgpx->links[*link].text, (const char *)xmlNodeGetContent(deeperNode), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
    } else if(!xmlStrcmp(deeperNode->name, (const xmlChar *)"type")){
      libgpx__strcpy(sgpx->links[*link].type, (const char *)xmlNodeGetContent(deeperNode), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
    }
    deeperNode = deeperNode->next;
  }

  ++(*link);
  --(*xmlDepth);
}

static void libgpx_parse_metadata(xmlNode *node, libgpx_GPX *sgpx, int *xmlDepth, size_t *link){
  xmlNode *thisNode;
  xmlNode *deeperNode;

  /** Defend against amlformed or malicious XML */
  ++(*xmlDepth);
  if(*xmlDepth > LIBGPX__MAXIMUM_XML_DEPTH){
    --(*xmlDepth);
    return;
  }
  /** ----------------------------------------- */
  thisNode = node;
  while(thisNode != NULL){
    if(!xmlStrcmp(thisNode->name, (const xmlChar *)"name")){
      libgpx__strcpy(sgpx->metadata.name, (const char *)xmlNodeGetContent(thisNode), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
    }
    else if(!xmlStrcmp(thisNode->name, (const xmlChar *)"copyright")){
      if(xmlGetProp(thisNode, (const xmlChar *)"author") != NULL){
        libgpx__strcpy(sgpx->metadata.copyright.author, (const char *)xmlGetProp(thisNode, (const xmlChar *)"author"), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
      }
      deeperNode = thisNode->children;
      while (deeperNode != NULL){
        if(!xmlStrcmp(deeperNode->name, (const xmlChar *)"license")){
          libgpx__strcpy(sgpx->metadata.copyright.license, (const char *)xmlNodeGetContent(deeperNode), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
        } else if(!xmlStrcmp(deeperNode->name, (const xmlChar *)"year")){
          sgpx->metadata.copyright.year = libgpx__strtoi((const char *)xmlNodeGetContent(deeperNode));
        }
        deeperNode = deeperNode->next;
      }
    } else if(!xmlStrcmp(thisNode->name, (const xmlChar *)"author")){
      deeperNode = thisNode->children;
      while (deeperNode != NULL){
        if(!xmlStrcmp(deeperNode->name, (const xmlChar *)"name")){
          libgpx__strcpy(sgpx->metadata.author.name, (const char *)xmlNodeGetContent(deeperNode), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
        } else if(!xmlStrcmp(deeperNode->name, (const xmlChar *)"email")){
          if(xmlGetProp(deeperNode, (const xmlChar *)"id")){
            libgpx__strcpy(sgpx->metadata.author.email.id, (const char *)xmlGetProp(deeperNode, (const xmlChar *)"id"), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
          }
          if(xmlGetProp(deeperNode, (const xmlChar *)"domain")){
            libgpx__strcpy(sgpx->metadata.author.email.domain, (const char *)xmlGetProp(deeperNode, (const xmlChar *)"domain"), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
          }
        } else if(!xmlStrcmp(deeperNode->name, (const xmlChar *)"link")){
          libgpx_parse_link(deeperNode, sgpx, -2, xmlDepth, link);
        }
        deeperNode = deeperNode->next;
      }
    } else if(!xmlStrcmp(thisNode->name, (const xmlChar *)"link")){
      libgpx_parse_link(thisNode, sgpx, -1, xmlDepth, link);
    }
    thisNode = thisNode->next;
  }

  --(*xmlDepth);
}

static void libgpx_parse_point(xmlNode *node, libgpx_GPX *sgpx, int *xmlDepth, size_t *nodeID, size_t *collection, size_t *point, size_t *link, int depth){
  xmlNode *this_node;

  /** Defend against amlformed or malicious XML */
  ++(*xmlDepth);
  if(*xmlDepth > LIBGPX__MAXIMUM_XML_DEPTH){
    --(*xmlDepth);
    return;
  }
  /** ----------------------------------------- */

  this_node = NULL;
  if(depth < 1){
    
    sgpx->points[*point].id = *nodeID;
    if(depth == 0){
      sgpx->points[*point].member_of = *collection;
    } else if(depth == -1) {
      sgpx->points[*point].member_of = -1;
    }

    char *s = &sgpx->points[*point].name[0];
    *s = '\0';
    s = &sgpx->points[*point].comment[0];
    *s = '\0';
    s = &sgpx->points[*point].description[0];
    *s = '\0';
    s = &sgpx->points[*point].source[0];
    *s = '\0';
    s = &sgpx->points[*point].symbol[0];
    *s = '\0';
    s = &sgpx->points[*point].fix[0];
    *s = '\0';
    s = &sgpx->points[*point].time[0];
    sgpx->points[*point].ageofdgpsdata = 0;

    if(xmlGetProp(node, (const xmlChar *)"lat")){
      sgpx->points[*point].latitude = libgpx__strtod((char *)xmlGetProp(node, (const xmlChar *)"lat"));
    }

    if(xmlGetProp(node, (const xmlChar *)"lon")){
      sgpx->points[*point].longitude = libgpx__strtod((char *)xmlGetProp(node, (const xmlChar *)"lon"));
    }

    libgpx_parse_point(node->children, sgpx, xmlDepth, nodeID, collection, point, link, 1);
  } else {
    this_node = node;
    while(this_node != NULL){
      if(this_node->type == XML_ELEMENT_NODE){
        if(!xmlStrcmp(this_node->name, (const xmlChar *)"ele")){
          sgpx->points[*point].elevation = libgpx__strtod((const char *)xmlNodeGetContent(this_node));
        } else if (!xmlStrcmp(this_node->name, (const xmlChar *)"name")){
          libgpx__strcpy(sgpx->points[*point].name, (const char*)xmlNodeGetContent(this_node), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
        } else if (!xmlStrcmp(this_node->name, (const xmlChar *)"cmt")){
          libgpx__strcpy(sgpx->points[*point].comment, (const char*)xmlNodeGetContent(this_node), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
        } else if (!xmlStrcmp(this_node->name, (const xmlChar *)"desc")){
          libgpx__strcpy(sgpx->points[*point].description, (const char*)xmlNodeGetContent(this_node), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
        } else if (!xmlStrcmp(this_node->name, (const xmlChar *)"src")){
          libgpx__strcpy(sgpx->points[*point].source, (const char*)xmlNodeGetContent(this_node), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
        } else if (!xmlStrcmp(this_node->name, (const xmlChar *)"sym")){
          libgpx__strcpy(sgpx->points[*point].symbol, (const char*)xmlNodeGetContent(this_node), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
        } else if (!xmlStrcmp(this_node->name, (const xmlChar *)"fix")){
          libgpx__strcpy(sgpx->points[*point].fix, (const char*)xmlNodeGetContent(this_node), 5);
        } else if (!xmlStrcmp(this_node->name, (const xmlChar *)"time")){
          libgpx__strcpy(sgpx->points[*point].time, (const char*)xmlNodeGetContent(this_node), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
        } else if(!xmlStrcmp(this_node->name, (const xmlChar *)"link")){
          libgpx_parse_link(this_node, sgpx, *nodeID, xmlDepth, link);
        } else if(!xmlStrcmp(this_node->name, (const xmlChar *)"ageofdgpsdata")){
          sgpx->points[*point].ageofdgpsdata = libgpx__strtod((const char*)xmlNodeGetContent(this_node));
        } else if(!xmlStrcmp(this_node->name, (const xmlChar *)"geoidheight")){
          sgpx->points[*point].geoidheight = libgpx__strtod((const char*)xmlNodeGetContent(this_node));
        } else if(!xmlStrcmp(this_node->name, (const xmlChar *)"hdop")){
          sgpx->points[*point].hdop = libgpx__strtod((const char*)xmlNodeGetContent(this_node));
        } else if(!xmlStrcmp(this_node->name, (const xmlChar *)"vdop")){
          sgpx->points[*point].vdop = libgpx__strtod((const char*)xmlNodeGetContent(this_node));
        } else if(!xmlStrcmp(this_node->name, (const xmlChar *)"pdop")){
          sgpx->points[*point].pdop = libgpx__strtod((const char*)xmlNodeGetContent(this_node));
        } else if(!xmlStrcmp(this_node->name, (const xmlChar *)"magvar")){
          sgpx->points[*point].magvar = libgpx__strtod((const char*)xmlNodeGetContent(this_node));
        } else if(!xmlStrcmp(this_node->name, (const xmlChar *)"dgpsid")){
          sgpx->points[*point].dgpsid = libgpx__strtoi((const char*)xmlNodeGetContent(this_node));
        } else if(!xmlStrcmp(this_node->name, (const xmlChar *)"satellite")){
          sgpx->points[*point].satellite = libgpx__strtod((const char*)xmlNodeGetContent(this_node));
        }
      }
      this_node = this_node->next;
    }
  }
  --(*xmlDepth);
  if(depth < 1){
    ++(*nodeID);
    ++(*point);
  }
}

static void libgpx_parse_collection(xmlNode* node, libgpx_GPX *sgpx, int *xmlDepth, unsigned int collection_type, int member_of, size_t *nodeID, size_t *collection, size_t *point, size_t *link){
  /** Defend against amlformed or malicious XML */
  ++(*xmlDepth);
  if(*xmlDepth > LIBGPX__MAXIMUM_XML_DEPTH){
    --(*xmlDepth);
    return;
  }
  /** ----------------------------------------- */

  xmlNode *thisNode = NULL;
  sgpx->collections[*collection].id = *nodeID;
  sgpx->collections[*collection].collection_type = collection_type;
  sgpx->collections[*collection].member_of = member_of;
  char *s = &sgpx->collections[*collection].name[0];
  *s = '\0';
  s = &sgpx->collections[*collection].type[0];
  *s = '\0';
  thisNode = node;
  while(thisNode){
    if(!xmlStrcmp(thisNode->name, (const xmlChar *)"name")){
      libgpx__strcpy(sgpx->collections[*collection].name, (const char*)xmlNodeGetContent(thisNode), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
    }
    else if(!xmlStrcmp(thisNode->name, (const xmlChar *)"type")){
      libgpx__strcpy(sgpx->collections[*collection].type, (const char*)xmlNodeGetContent(thisNode), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
    }
    else if(!xmlStrcmp(thisNode->name, (const xmlChar *)"trkseg")){
      ++(*collection);
      ++(*nodeID);
      libgpx_parse_collection(thisNode->children, sgpx, xmlDepth, LIBGPX__COLLECTION_TYPE_TRKSEG, *collection, nodeID, collection, point, link);
      --(*collection);
      --(*nodeID);
    }
    else if(!xmlStrcmp(thisNode->name, (const xmlChar *)"trkpt")){
      libgpx_parse_point(thisNode, sgpx, xmlDepth, nodeID, collection, point, link, 0);
    }
    else if(!xmlStrcmp(thisNode->name, (const xmlChar *)"link")){
      libgpx_parse_link(thisNode, sgpx, *nodeID, xmlDepth, link);
    }
    thisNode = thisNode->next;
  }

  --(*xmlDepth);
  ++(*collection);
  ++(*nodeID);
}

static void libgpx_parse_gpx_node(xmlNode* node, libgpx_GPX *sgpx, int *xmlDepth, size_t *nodeID, size_t *collection, size_t *point, size_t *link){
  /** Defend against amlformed or malicious XML */
  ++(*xmlDepth);
  if(*xmlDepth > LIBGPX__MAXIMUM_XML_DEPTH){
    --(*xmlDepth);
    return;
  }
  /** ----------------------------------------- */

  if(xmlGetProp(node, (const xmlChar *)"version") != NULL){
    sgpx->version = (float)libgpx__strtod((char *)xmlGetProp(node, (const xmlChar *)"version"));
  }

  if(xmlGetProp(node, (const xmlChar *)"creator") != NULL){
    libgpx__strcpy(sgpx->creator, (char *)xmlGetProp(node, (const xmlChar *)"creator"), LIBGPX__XML_STRING_ALLOCATED_LENGTH);
  }

  xmlNode *this_node = NULL;
  this_node = node;
  while(this_node != NULL){

    if (!xmlStrcmp(this_node->name, (const xmlChar *)"trk")){
      libgpx_parse_collection(this_node->children, sgpx, xmlDepth, LIBGPX__COLLECTION_TYPE_TRK, -1, nodeID, collection, point, link);
    }
    else if (!xmlStrcmp(this_node->name, (const xmlChar *)"metadata")){
      libgpx_parse_metadata(this_node->children, sgpx, xmlDepth, link);
    }
    else if (!xmlStrcmp(this_node->name, (const xmlChar *)"wpt")){
      libgpx_parse_point(this_node, sgpx, xmlDepth, nodeID, collection, point, link, -1);
    }
    else {
      libgpx_parse_gpx_node(this_node->children, sgpx, xmlDepth, nodeID, collection, point, link);
    }
    this_node = this_node->next;
  }
  --(*xmlDepth);
} 

void libgpx_parse_gpx(unsigned char* gpx, libgpx_GPX* sgpx){
  xmlDocPtr doc = xmlParseDoc(gpx);
  xmlNodePtr _gpx = xmlDocGetRootElement(doc);
  int xmlDepth = 0;
  int *xmlDepthPtr;
  size_t nodeID = 0;
  size_t *nodeIDPtr;
  size_t link = 0;
  size_t *linkPtr;
  size_t collection = 0;
  size_t *collectionPtr;
  size_t point = 0;
  size_t *pointPtr;
  
  xmlDepthPtr = &xmlDepth;
  nodeIDPtr = &nodeID;
  collectionPtr = &collection;
  pointPtr = &point;
  linkPtr = &link;

  /** Initialise the top level structure elements */
  char *s = &sgpx->metadata.name[0];
  *s = '\0';
  s = &sgpx->metadata.author.name[0];
  *s = '\0';
  s = &sgpx->metadata.author.email.domain[0];
  *s = '\0';
  s = &sgpx->metadata.author.email.id[0];
  *s = '\0';
  s = &sgpx->metadata.copyright.author[0];
  *s = '\0';
  s = &sgpx->metadata.copyright.license[0];
  *s = '\0';
  s = &sgpx->creator[0];
  *s = '\0';
  sgpx->metadata.copyright.year = (unsigned int)0;
  sgpx->version = (float)0;
  /** ------------------------------------------- */

  libgpx_parse_gpx_node(_gpx, sgpx, xmlDepthPtr, nodeIDPtr, collectionPtr, pointPtr, linkPtr);

  /** Store all list lengths */
  sgpx->n_GPXLink = link;
  sgpx->n_GPXPoint = point;
  sgpx->n_GPXPointCollection = collection;
  /** ---------------------- */
}

void libgpx_bounding_box_from_gpx_object(libgpx_GPX* sgpx, libgpx_BoundingBox* box){

  float max_ele = -20000;
  if(max_ele){
    float min_ele = 20000;
    if(min_ele){
      float max_lat = -90;
      float min_lat = 90;
      float max_lon = -180;
      float min_lon = 180;

      int i = 0;
      while(i<sgpx->n_GPXPoint){
        if(sgpx->points[i].latitude < min_lat){
          min_lat = sgpx->points[i].latitude;
        }
        if(sgpx->points[i].longitude < min_lon){
          min_lon = sgpx->points[i].longitude;
        }
        if(sgpx->points[i].elevation < min_ele){
          min_ele = sgpx->points[i].elevation;
        }
        if(sgpx->points[i].elevation > max_ele){
          max_ele = sgpx->points[i].elevation;
        }
        if(sgpx->points[i].latitude > max_lat){
          max_lat = sgpx->points[i].latitude;
        }
        if(sgpx->points[i].longitude > max_lon){
          max_lon = sgpx->points[i].longitude;
        }
        ++i;
      }
      box->min_ele = min_ele;
      box->min_lat = min_lat;
      box->min_lon = min_lon;
      box->max_ele = max_ele;
      box->max_lat = max_lat;
      box->max_lon = max_lon;
    }
  }

}