/**
 * Geometries are built from a given list of nodes at a given timestamp.
 * This class builds a geos geometry from this information, depending
 * on the tags, a way could possibly be a polygon. This information is
 * added additionally when building a portugal.
 */

#ifndef IMPORTER_GEOMBUILDER_HPP
#define IMPORTER_GEOMBUILDER_HPP

#include "project.hpp"

class GeomBuilder {
private:
    Nodestore *m_nodestore;
    DbAdapter *m_adapter;
    bool m_isupdate, m_keepLatLng;
    bool m_debug, m_showerrors;
    geos::geom::GeometryFactory* geos_factory() {
        static std::unique_ptr<const geos::geom::PrecisionModel> precision_model{new geos::geom::PrecisionModel};
        static std::unique_ptr<geos::geom::GeometryFactory> factory{new geos::geom::GeometryFactory(precision_model.get(), -1)};
        return factory.get();
    }

protected:
    GeomBuilder(Nodestore *nodestore, DbAdapter *adapter, bool isUpdate): m_nodestore(nodestore), m_adapter(adapter), m_isupdate(isUpdate), m_debug(false), m_showerrors(false) {}

public:

    geos::geom::Geometry* forWay(const osmium::WayNodeList &nodes, time_t t, bool looksLikePolygon) {
        // shorthand to the geometry factory
        //geos::geom::GeometryFactory *f = Osmium::Geometry::geos_geometry_factory();

        // pointer to coordinate vector
        std::vector<geos::geom::Coordinate> *c = new std::vector<geos::geom::Coordinate>();

        // iterate over all nodes
        //osmium::WayNodeList::const_iterator end = nodes.end();
        //for(osmium::WayNodeList::const_iterator it = nodes.begin(); it != end; ++it) {
        for (const auto& node_ref : nodes) {
            // the id
            //osmium::object_id_type id = it->ref();
            osmium::object_id_type id = node_ref.ref();

            // was the node found in the store?
            bool found;
            Nodestore::Nodeinfo info = m_nodestore->lookup(id, t, found);

            // a missing node can just be skipped
            if(!found)
                continue;

            double lon = info.lon, lat = info.lat;

            if(m_debug) {
                std::cerr << "node #" << id << " at tstamp " << t << " references node at POINT(" << std::setprecision(8) << lon << ' ' << lat << ')' << std::endl;
            }

            // create a coordinate-object and add it to the vector
            if(!m_keepLatLng) {
                if(!Project::toMercator(&lon, &lat))
                    continue;
            }
            c->push_back(geos::geom::Coordinate(lon, lat, DoubleNotANumber));
        }

        // if less then 2 nodes could be found in the store, no valid way
        // can be assembled and we need to skip it
        if(c->size() < 2) {
            if(m_showerrors) {
                std::cerr << "found only " << c->size() << " valid coordinates, skipping way" << std::endl;
            }
            delete c;
            return NULL;
        }

        // the resulting geometry
        geos::geom::Geometry* geom;

        // geos throws exception on bad geometries and such
        try {
            // tags say it could be a polygon, the way is closed and has
            // at least 3 *different* coordinates
            if(looksLikePolygon && c->front() == c->back() && c->size() >= 4) {
                // build a polygon
                geom = geos_factory()->createPolygon(
                    geos_factory()->createLinearRing(
                        geos_factory()->getCoordinateSequenceFactory()->create(c)
                    ),
                    NULL
                );
            } else {
                // build a linestring
                geom = geos_factory()->createLineString(
                    geos_factory()->getCoordinateSequenceFactory()->create(c)
                );
            }
        } catch(geos::util::GEOSException e) {
            if(m_showerrors) {
                std::cerr << "error creating polygon: " << e.what() << std::endl;
            }
            delete c;
            return NULL;
        }

        // enforce srid
        if(geom) {
            geom->setSRID(900913);
        }

        return geom;
    }

    bool isKeepingLatLng() {
        return m_keepLatLng;
    }

    void keepLatLng(bool shouldKeepLatLng) {
        m_keepLatLng = shouldKeepLatLng;
    }

    /**
     * is this nodestore printing debug messages
     */
    bool isPrintingDebugMessages() {
        return m_debug;
    }

    /**
     * should this nodestore print debug messages
     */
    void printDebugMessages(bool shouldPrintDebugMessages) {
        m_debug = shouldPrintDebugMessages;
    }
};

class ImportGeomBuilder : public GeomBuilder {
public:
    ImportGeomBuilder(Nodestore *nodestore, DbAdapter *adapter) : GeomBuilder(nodestore, adapter, false) {}
};

class UpdateGeomBuilder : public GeomBuilder {
public:
    UpdateGeomBuilder(Nodestore *nodestore, DbAdapter *adapter) : GeomBuilder(nodestore, adapter, true) {}
};

#endif // IMPORTER_GEOMBUILDER_HPP
