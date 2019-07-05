#ifndef BVH_OBJECT_H
#define BVH_OBJECT_H

#include "hitable.h"
#include "hitable_list.h"
#include "bvh_node.h"
#include "bvh_manager.h"
#include "ray.h"
#include <vector>
#include <utility>

class bvh_object : public hitable
{
public:
	bvh_object(std::vector<std::unique_ptr<hitable>> objs)
	{
		for (auto& obj : objs) {
			m_manager.add(std::move(obj));
		}
		m_manager.build_scene();
	}

	bool hit(const ray3& r, float tmin, float tmax, hit_record& rec) const override
	{
		static thread_local std::vector<object*> results;

		m_manager.root().raycast(r, results);
		bool was_hit = check_hit(results, r, tmin, tmax, rec);
		results.clear();

		return was_hit;
	}

	aabb3 get_aabb() const override
	{
		return static_cast<const bvh_node&>(m_manager.root()).get_aabb();
	}
private:
	bvh_manager m_manager;
};

#endif
