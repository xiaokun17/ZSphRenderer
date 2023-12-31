#pragma once

#include "CNDataStructure.h"

#include <vector>

namespace CompactNSearch
{
	class NeighborhoodSearch;

	/**
	* @class PointSet.
	* Represents a set of points in three-dimensional space.
	*/
	class PointSet
	{

	public:

		/**
		* Copy constructor.
		*/
		PointSet(PointSet const& other)
		{
			*this = other;
		}

		/**
		* Assignment operator.
		*/
		PointSet& operator=(PointSet const& other)
		{
			m_x = other.m_x;
			m_n = other.m_n;
			m_dynamic = other.m_dynamic;
			m_search_neighbors = other.m_search_neighbors;

			m_neighbors = other.m_neighbors;
			m_keys = other.m_keys;
			m_old_keys = other.m_old_keys;
			m_locks.resize(other.m_locks.size());
			m_sort_table = other.m_sort_table;

			return *this;
		}

		/**
		* Returns the number of neighbors of point i in the given point set.
		* @param i Point index.
		* @returns Number of points neighboring point i.
		*/
		std::size_t n_neighbors(unsigned int i) const
		{
			return static_cast<unsigned int>(m_neighbors[i].size());
		}

		/**
		* Fetches id pair of kth neighbor of point i in the given point set.
		* @param i Point index for which the neighbor id should be returned.
		* @param k Represents kth neighbor of point i.
		* @returns Number of points neighboring point i.
		*/
		PointID const& neighbor(unsigned int i, unsigned int k) const
		{
			return m_neighbors[i][k];
		}

		/**
		* Returns the number of points contained in the point set.
		*/
		std::size_t n_points() const { return m_n; }

		/*
		* Returns true, if the point locations may be updated by the user.
		**/
		bool is_dynamic() const { return m_dynamic; }

		/**
		* If true is passed, the point positions may be altered by the user.
		*/
		void set_dynamic(bool v) { m_dynamic = v; }

		/**
		* Returns true, if neighborhood information will be generated for the given point set.
		*/
		bool is_neighborsearch_enabled() const { return m_search_neighbors; }

		/**
		* Enables or disables, if neighborhood information will be generated for the given point set.
		*/
		void enable_neighborsearch(bool v) { m_search_neighbors = v; }

		/**
		* Reorders an array according to a previously generated sort table by invocation of the method
		* "z_sort" of class "NeighborhoodSearch". Please note that the method "z_sort" of class
		* "Neighborhood search" has to be called beforehand.
		*/
		template <typename T>
		void sort_field(T* lst) const;

	private:

		friend NeighborhoodSearch;
		PointSet(Real const* x, std::size_t n, bool dynamic, bool search_neighbors)
			: m_x(x), m_n(n), m_dynamic(dynamic), m_neighbors(n)
			, m_keys(n, {
			std::numeric_limits<int>::lowest(),
			std::numeric_limits<int>::lowest(),
			std::numeric_limits<int>::lowest() })
			, m_search_neighbors(search_neighbors)
		{
			m_old_keys = m_keys;
		}

		void resize(Real const* x, std::size_t n)
		{
			m_x = x;
			m_n = n;
			m_keys.resize(n, {
				std::numeric_limits<int>::lowest(),
				std::numeric_limits<int>::lowest(),
				std::numeric_limits<int>::lowest() });
			m_old_keys.resize(n, {
				std::numeric_limits<int>::lowest(),
				std::numeric_limits<int>::lowest(),
				std::numeric_limits<int>::lowest() });
			m_neighbors.resize(n);
		}

		Real const* point(unsigned int i) const { return &m_x[3 * i]; }

	private:

		Real const* m_x;
		std::size_t m_n;
		bool m_dynamic;
		bool m_search_neighbors;

		std::vector<std::vector<PointID>> m_neighbors;
		std::vector<HashKey> m_keys, m_old_keys;
		std::vector<Spinlock> m_locks;
		std::vector<unsigned int> m_sort_table;
	};

	template <typename T> void
		PointSet::sort_field(T* lst) const
	{
		if (m_sort_table.empty())
		{
			std::cerr << "WARNING: No sort table was generated for the current point set. "
				<< "First invoke the method 'z_sort' of the class 'NeighborhoodSearch.'" << std::endl;
			return;
		}

		std::vector<T> tmp(lst, lst + m_sort_table.size());
		std::transform(m_sort_table.begin(), m_sort_table.end(),
#ifdef _MSC_VER
			stdext::unchecked_array_iterator<T*>(lst),
#else
			lst,
#endif
			[&](int i) { return tmp[i]; });
	}
}