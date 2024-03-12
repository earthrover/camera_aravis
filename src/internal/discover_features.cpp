
#include <camera_aravis_internal/aravis_abstraction.h>
#include <unordered_map>
#include <unordered_set>
#include <list>

namespace camera_aravis::internal {

    std::unordered_map<std::string, const bool> discover_features(const NonOwnedGPtr<ArvDevice>& device) {
        std::unordered_map<std::string, const bool> implemented_features;

        if (!device) return implemented_features;

        // get the root node of genicam description
        ArvGc* gc = arv_device_get_genicam(device.get());
        if (!gc) return implemented_features;

        std::unordered_set<ArvDomNode*> done;
        std::list<ArvDomNode*> todo;

        todo.push_front((ArvDomNode*) arv_gc_get_node(gc, "Root"));

        while (!todo.empty()) {
            // get next entry
            ArvDomNode* node = todo.front();
            todo.pop_front();

            if (done.find(node) != done.end()) continue;
            done.insert(node);

            const std::string name(arv_dom_node_get_node_name(node));

            // Do the indirection
            if (name[0] == 'p') {
                if (name.compare("pInvalidator") == 0) { continue; }
                ArvDomNode* inode =
                    (ArvDomNode*) arv_gc_get_node(gc, arv_dom_node_get_node_value(arv_dom_node_get_first_child(node)));
                if (inode) todo.push_front(inode);
                continue;
            }

            // check for implemented feature
            if (ARV_IS_GC_FEATURE_NODE(node)) {
                ArvGcFeatureNode* fnode = ARV_GC_FEATURE_NODE(node);
                const std::string fname(arv_gc_feature_node_get_name(fnode));
                const bool usable =
                    arv_gc_feature_node_is_available(fnode, NULL) && arv_gc_feature_node_is_implemented(fnode, NULL);

                implemented_features.emplace(fname, usable);
            }
            if (ARV_IS_GC_CATEGORY(node)) {
                const GSList* features;
                const GSList* iter;
                features = arv_gc_category_get_features(ARV_GC_CATEGORY(node));
                for (iter = features; iter != NULL; iter = iter->next) {
                    ArvDomNode* next = (ArvDomNode*) arv_gc_get_node(gc, (const char*) iter->data);
                    todo.push_front(next);
                }

                continue;
            }

            // add children in todo-list
            ArvDomNodeList* children = arv_dom_node_get_child_nodes(node);
            const uint l = arv_dom_node_list_get_length(children);
            for (uint i = 0; i < l; ++i) { todo.push_front(arv_dom_node_list_get_item(children, i)); }
        }

        return implemented_features;
    }
}  // namespace camera_aravis::internal
