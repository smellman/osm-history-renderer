/**
 * The handler always needs to know what the next node/way/relation
 * in the file lookes like to answer questions like "what is the
 * valid_to date of the current entity" or "is this the last version
 * of that entity". It also sometimes needs to know hot the previous
 * entity looks like to answer questions like "was this an area or a line
 * before it got deleted". The EntityTracker takes care of keeping the
 * previous, current and next entity, frees them as required and does
 * basic comparations.
 */

#ifndef IMPORTER_ENTITYTRACKER_HPP
#define IMPORTER_ENTITYTRACKER_HPP

/**
 * Dummy object to wrap object reference.
 */
template <class TObject>
class ObjectWrapper {
    const TObject& object;
public:
    ObjectWrapper(const TObject& ref) : object(ref) {
        //std::cout << "Object: " << object.id() << std::endl;
    }

    const TObject& obj() {
        return object;
    }
};

/**
 * Tracks the previous, the current and the next entity, provides
 * a method to shift the entities into the next state and manages
 * freeing of the entities. It is templated to allow nodes, ways
 * and relations as child objects.
 */
template <class TObject>
class EntityTracker {

private:
    /**
     * object of the current entity
     */
    ObjectWrapper<TObject> *m_prev;
    //osmium::object_id_type m_prev_id;

    /**
     * object of the current entity
     */
    ObjectWrapper<TObject> *m_cur;
    //osmium::object_id_type m_cur_id;

    /**
     * object of the next entity
     */
    ObjectWrapper<TObject> *m_next;
    //osmium::object_id_type m_next_id;

public:
    EntityTracker() {

        m_prev = nullptr;
        m_cur = nullptr;
        m_next = nullptr;

        /*
        m_prev_id = 0;
        m_cur_id = 0;
        m_next_id = 0;
        */
    }

    /**
     * get the reference to the previous entity
     */
    const TObject& prev() {
        return m_prev->obj();
    }

    /**
     * get the reference to the current entity
     */
    const TObject& cur() {
        return m_cur->obj();
    }

    /**
     * get the reference to the next entity
     */
    const TObject& next() {
        return m_next->obj();
    }

    /**
     * returns if the tracker currently tracks a previous entity
     */
    bool has_prev() {
        return m_prev != nullptr;
        //return m_prev;
    }

    /**
     * returns if the tracker currently tracks a current entity
     */
    bool has_cur() {
        return m_cur != nullptr;
        //return m_cur;
    }

    /**
     * returns if the tracker currently tracks a "next" entity
     */
    bool has_next() {
        return m_next != nullptr;
        //return m_next;
    }

    /**
     * returns if the tracker currently tracks a "current" and a "previous"
     * entity with the same id
     */
    bool prev_is_same_entity() {
        return has_cur() && has_prev() && (cur().id() == prev().id());
    }

    /**
     * returns if the tracker currently tracks a "current" and a "next"
     * entity with the same id
     */
    bool next_is_same_entity() {
        return has_cur() && has_next() && (cur().id() == next().id());
    }

    /**
     * feed in a new object as the next one
     *
     * if a next one still exists, the program will abort with an
     * assertation error, because the next enity needs to be swapped
     * away using the swap-method below, before feeding in a new one.
     */
    void feed(const TObject& obj) {
        assert(!m_next);
        m_next = new ObjectWrapper<TObject>(obj);
        //m_next_id = m_next->obj().id();
        //std::cout << "feed m_prev_id: " << m_prev_id << " m_cur_id: " << m_cur_id << " m_next_id: " << m_next_id << std::endl;
    }

    /**
     * copy the current entity to previous and the next entity to current.
     * clear the next entity pointer
     */
    void swap() {
        //m_prev = m_cur;
        delete m_prev;

        if (m_cur != nullptr) {
            m_prev = new ObjectWrapper<TObject>(m_cur->obj());
        } else {
            m_prev = nullptr;
        }
        /*
        if (m_prev != nullptr) {
            m_prev_id = m_prev->obj().id();
        }*/
        delete m_cur;
        //m_cur = m_next;

        if (m_next != nullptr) {
            m_cur = new ObjectWrapper<TObject>(m_next->obj());
        } else {
            m_cur = nullptr;
        }
        /*
        if (m_cur != nullptr) {
            m_cur_id = m_cur->obj().id();
        }
        */
        delete m_next;
        m_next = nullptr;
        //m_next_id = 0;
        //std::cout << "swap m_prev_id: " << m_prev_id << " m_cur_id: " << m_cur_id << " m_next_id: " << m_next_id << std::endl;
    }
};

#endif // IMPORTER_ENTITYTRACKER_HPP
