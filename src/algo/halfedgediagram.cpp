/*  $Id$
 * 
 *  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of OpenCAMlib.
 *
 *  OpenCAMlib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenCAMlib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "halfedgediagram.h"

namespace ocl
{

int VertexProps::count = 0;

void VertexProps::set_position() {
    double w = J4;
    double x = - J2/w+pk.x;
    double y = J3/w+pk.y;
    position =  Point(x,y);
}

double VertexProps::detH(const Point& pl) {
    H = J2*(pl.x-pk.x) - J3*(pl.y-pk.y) + 0.5*J4*(square(pl.x-pk.x) + square(pl.y-pk.y));
    return H;
}

void VertexProps::set_J(Point& pi, Point& pj, Point& pkin) { 
    // 1) i-j-k should come in CCW order
    Point pi_,pj_,pk_;
    if ( pi.isRight(pj,pkin) ) {
        pi_ = pj;
        pj_ = pi;
        pk_ = pkin;
    } else {
        pi_ = pi;
        pj_ = pj;
        pk_ = pkin;
    }
    assert( !pi_.isRight(pj_,pk_) );
    // 2) point pk should have the largest angle 
    // largest angle is opposite longest side.
    Point pi__,pj__,pk__;
    pi__ = pi_;                          
    pj__ = pj_;                          
    pk__ = pk_;
    double longest_side = (pi_ - pj_).xyNorm();
    if (  (pj_ - pk_).xyNorm() > longest_side ) {
        longest_side = (pj_ - pk_).xyNorm(); //j-k is longest, so i should be new k
        pk__ = pi_;                         // old  i-j-k 
        pi__ = pj_;                         // new  k-i-j
        pj__ = pk_;
    }
    if ( (pi_ - pk_).xyNorm() > longest_side ) { // i-k is longest, so j should be new k                    
        pk__ = pj_;                          // old  i-j-k
        pj__ = pi_;                          // new  j-k-i
        pi__ = pk_;
    }
    
    assert( !pi__.isRight(pj__,pk__) );
    assert( (pi__ - pj__).xyNorm() >=  (pj__ - pk__).xyNorm() );
    assert( (pi__ - pj__).xyNorm() >=  (pk__ - pi__).xyNorm() );
    
    this->pk = pk__;
    J2 = detH_J2( pi__, pj__, pk__);
    J3 = detH_J3( pi__, pj__, pk__);
    J4 = detH_J4( pi__, pj__, pk__);

    //assert( J4 > 0.0 );

}
/// the J2 determinant
double VertexProps::detH_J2(Point& pi, Point& pj, Point& pk) {
    return (pi.y-pk.y)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.y-pk.y)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
}
/// the J3 determinant
double VertexProps::detH_J3(Point& pi, Point& pj, Point& pk) {
    return (pi.x-pk.x)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.x-pk.x)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
}
/// the J4 determinant
double VertexProps::detH_J4(Point& pi, Point& pj, Point& pk) {
    return (pi.x-pk.x)*(pj.y-pk.y) - (pj.x-pk.x)*(pi.y-pk.y);
}




/* ****************** HalfEdgeDiagram ******************************** */
HalfEdgeDiagram::HalfEdgeDiagram() {
    // do nothing.
}

HEEdge HalfEdgeDiagram::add_edge(HEVertex v1, HEVertex v2) {
    HEEdge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, *this);
    return e;
}

HEEdge HalfEdgeDiagram::add_edge(HEVertex v1, HEVertex v2, EdgeProps prop) {
    HEEdge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, prop, *this);
    return e;
}

HEFace HalfEdgeDiagram::add_face( FaceProps f_prop ) {
    faces.push_back(f_prop);
    HEFace index = faces.size()-1;
    faces[index].idx = index;
    return index;    
}

EdgeVector HalfEdgeDiagram::edges() const {
    EdgeVector ev;
    HEEdgeItr it, it_end;
    boost::tie( it, it_end ) = boost::edges( *this );
    for ( ; it != it_end ; ++it ) {
        ev.push_back(*it);
    }
    return ev;
}
        
EdgeVector HalfEdgeDiagram::out_edges( HEVertex v ) const {
    EdgeVector ev;
    HEOutEdgeItr it, it_end;
    boost::tie( it, it_end ) = boost::out_edges( v, *this );
    for ( ; it != it_end ; ++it ) {
        ev.push_back(*it);
    }
    return ev;
}

VertexVector HalfEdgeDiagram::vertices() const {
    VertexVector vv;
    HEVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( *this );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        vv.push_back( *itr );
    }
    return vv;
}

HEEdge HalfEdgeDiagram::previous_edge(HEEdge e) {
    HEEdge previous = (*this)[e].next;
    while ( (*this)[previous].next != e ) {
        previous = (*this)[previous].next;
    }
    return previous;
}

VertexVector HalfEdgeDiagram::adjacent_vertices(HEVertex v) const {
    VertexVector vv;
    BOOST_FOREACH( HEEdge edge, this->out_edges( v ) ) {
        vv.push_back( this->target( edge ) );
    }
    return vv;
}

// traverse face and return all vertices found
VertexVector HalfEdgeDiagram::face_vertices(HEFace face_idx) const {
    VertexVector verts;
    HEEdge startedge = faces[face_idx].edge; // the edge where we start
    HEVertex start_target = boost::target( startedge, *this); 
    verts.push_back(start_target);
    HEEdge current = (*this)[startedge].next;
    do {
        HEVertex current_target = boost::target( current, *this); 
        assert( current_target != start_target );
        verts.push_back(current_target);
        current = (*this)[current].next;
    } while ( current != startedge );
    return verts;
}
FaceVector HalfEdgeDiagram::adjacent_faces( HEVertex q ) {
    // given the vertex q, find the three adjacent faces
    std::set<HEFace> face_set;
    HEOutEdgeItr itr, itr_end;
    boost::tie( itr, itr_end) = boost::out_edges(q, *this);
    for ( ; itr!=itr_end ; ++itr ) {
        face_set.insert( (*this)[*itr].face );
    }
    //assert( face_set.size() == 3); // degree of q is three, so has three faces
    FaceVector fv;
    BOOST_FOREACH(HEFace m, face_set) {
        fv.push_back(m);
    }
    return fv;
}

/// insert vertex v into edge e
void HalfEdgeDiagram::insert_vertex_in_edge(HEVertex v, HEEdge e) {
    // the vertex v is in the middle of edge e
    //                    face
    //                    e1   e2
    // previous-> source  -> v -> target -> next
    //            tw_trg  <- v <- tw_src <- tw_previous
    //                    te2  te1
    //                    twin_face
    
    HEEdge twin = (*this)[e].twin;
    HEVertex source = boost::source( e , *this );
    HEVertex target = boost::target( e , *this );
    HEVertex twin_source = boost::source( twin , *this );
    HEVertex twin_target = boost::target( twin , *this );
    assert( source == twin_target );    
    assert( target == twin_source );
    
    HEFace face = (*this)[e].face;
    HEFace twin_face = (*this)[twin].face;
    HEEdge previous = previous_edge(e);
    assert( (*this)[previous].face == (*this)[e].face );
    HEEdge twin_previous = previous_edge(twin);
    assert( (*this)[twin_previous].face == (*this)[twin].face );
    
    HEEdge e1 = add_edge( source, v  );
    HEEdge e2 = add_edge( v, target  );
    
    // preserve the left/right face link
    (*this)[e1].face = face;
    (*this)[e2].face = face;
    // next-pointers
    (*this)[previous].next = e1;
    (*this)[e1].next = e2;
    (*this)[e2].next = (*this)[e].next;
    
    
    HEEdge te1 = add_edge( twin_source, v  );
    HEEdge te2 = add_edge( v, twin_target  );
    
    (*this)[te1].face = twin_face;
    (*this)[te2].face = twin_face;
    
    (*this)[twin_previous].next = te1;
    (*this)[te1].next = te2;
    (*this)[te2].next = (*this)[twin].next;
    
    // TWINNING (note indices 'cross', see ASCII art above)
    (*this)[e1].twin = te2;
    (*this)[te2].twin = e1;
    (*this)[e2].twin = te1;
    (*this)[te1].twin = e2;
    
    // update the faces (required here?)
    (*this)[face].edge = e1;
    (*this)[twin_face].edge = te1;
    
    // finally, remove the old edge
    boost::remove_edge( e   , (*this));
    boost::remove_edge( twin, (*this));
}


} // end namespace
// end file halfedgediagram.cpp
