#pragma once

#include <iostream>
#include <map>
#include <cmath>
#include <vector>

#include "loggers.hpp"

namespace ioh {
namespace logger {

    /** Implementation details of the logger::EAH class. */
    namespace eah {

        /** @defgroup EAH Empirical Attainment Histogram
         * Features related to the EAH logger.
         * 
         * @ingroup Loggers
         */

        /** @defgroup EAH_API EAH API
         * API related to the EAH logger.
         * 
         * @ingroup Logging
         */

        /** Interface for classes that computes indices from ranges.
         *
         * The idea is to input [min,max] and the discretization size
         * and to compute the position in [0,size-1] of any value in [min,max].
         * 
         * You can think of it as a way to compute discrete histograms' bucket indices.
         * Or as a morphism between any 1D "origin" space to another 1D strictly positive integer "indexed" space.
         * 
         * @warning Contrary to what is common in computer science,
         *          this uses a closed [min,max] interval for the "origin" values.
         *          So that any value at `max` will be assigned to the `size-1`th bucket.
         *
         * Used in EAH.
         * 
         * @ingroup EAH_API
         */
         // Why a closed interval? Because it's more natural for the user to indicates the number of evaluations that way.
        template <class R>
        class Scale {
        public:

            /** Constructor.
             * 
             * @param amin the minimum of the scale domain.
             * @param amax the maximum of the scale domain.
             * @param asize the number of buckets in the scale.
             */
            Scale(const R amin, const R amax, const size_t asize)
                : _min(amin)
                , _max(amax)
                , _size(asize)
            {
                assert(_min < _max);
                assert(0 < _size);
            }

            /** Minimum value on the axis. */
            R min() const { return _min; }

            /** Maximum value on the axis. */
            R max() const { return _max; }

            /** Number of buckets. */
            size_t size() const { return _size; }

            /** Span of the axis. */
            R length() const { return _max - _min; }

            /** Interface: from value to index.
             * 
             * @param x The targeted real value.
             * @returns The corresponding index.
             */
            virtual size_t index(const double x) const = 0;

            /** Type returned by the bounds method.
             * 
             * @note This is a pair of double, whatever the type of the Scale,
             *       because one can ask for non-integer buckets limits
             *       (e.g. when size > max-min).
             */
            using BoundsType = std::pair<double,double>;

            /** Interface: from index to bounds values.
             * 
             * @param i Targeted index.
             * @returns The {min,max} pair of the bounds of the corresponding bucket.
             */
            virtual BoundsType bounds(const size_t i) const = 0;

        protected:
            //! Minimum.
            R _min;
            //! Maximum.
            R _max;
            //! Number of buckets.
            size_t _size;
        };

        /** Linear scale.
         * @ingroup EAH
         */
        template <class R>
        class LinearScale : public Scale<R> {
        public:
            /** Constructor.
             * 
             * @param min the minimum of the scale domain.
             * @param max the maximum of the scale domain.
             * @param size the number of buckets in the scale.
             */
            LinearScale(const R min, const R max, const size_t size)
                : Scale<R>(min, max, size)
            {
            }

            /** Returns the index of the bucket in which a value falls.
             *
             * @param x the value on the scale.
             * @returns the corresponding bucket index.
             */
            size_t index(const double x) const override
            {
                assert(this->_min <= x);
                assert(x <= this->_max); // FIXME do we want that?
                if(x >= this->max()) {
                    return this->size() - 1;
                } else {
                    return std::floor(
                        (x - this->min()) / this->length() * this->size());
                }
            }

            //! The type in which bounds are stored by the `bounds` method.
            using BoundsType = typename Scale<R>::BoundsType;

            /** Returns the bounds of the corresponding bucket index.
             *
             * @param i the index of the bucket.
             * @returns [min,max[ of the bucket i.
             */
            BoundsType bounds(const size_t i) const override
            {
                assert(i < this->size());
                const double w = this->step();
                const double m = static_cast<double>(i) / this->size() * this->length() + this->min();
                assert(m < m+w);
                return std::make_pair(m, m + w);
            }

            /** Returns the width of a bucket. */
            R step() const { return this->length() / this->_size; }
        };

        /** Logarithmic (base 2) scale. 
         * 
         * @warning This implementation is not very robust to rounding errors
         *          especially if you use the bounds method with large lengths.
         * 
         * @ingroup EAH
         */
        template<class R>
        class Log2Scale : public Scale<R> {
        protected:
            //! The minimum.
            const double _m;
            //! Max - min.
            const double _l;
            //! The number of buckets.
            const double _s;
            //! The `log(1+max-min)` constant.
            const double _k;

        public:
            /** Constructor.
             * 
             * @param min the minimum of the scale domain.
             * @param max the maximum of the scale domain.
             * @param size the number of buckets in the scale.
             */
            Log2Scale(const R min, const R max, const size_t size)
                : Scale<R>(min, max, size),
                _m(min),
                _l(max-min),
                _s(size),
                _k( std::log2(1+max-min) )
            {
                // FIXME check size against max-min ?
            }

            /** Returns the index of the bucket in which a value falls.
             *
             * @param x the value on the scale.
             * @returns the corresponding bucket index.
             */
            size_t index(const double x) const override
            {
                assert(this->_min <= x);
                assert(x <= this->max()); // FIXME do we want that?
                if(x >= this->max()) {
                    return this->size() - 1;
                } else {
                    return static_cast<size_t>(
                        std::floor( std::log2(1 + (x - _m)) / _k * _s )
                    );
                }
            }

            //! The type in which bounds are stored by the `bounds` method.
            using BoundsType = typename Scale<R>::BoundsType;

            /** Returns the bounds of the corresponding bucket index.
             *
             * @param i the index of the bucket.
             * @returns [min,max[ of the bucket i.
             */
            BoundsType bounds(const size_t i) const override
            {
                assert(i < _s);
                const double m = std::pow(2,  static_cast<double>(i)        * std::log2(1.0 + _l) / _s) - 1.0 + _m;
                const double M = std::pow(2, (static_cast<double>(i) + 1.0) * std::log2(1.0 + _l) / _s) - 1.0 + _m;
                assert(m < M);
                assert(0 < M-m);
                assert(M-m <= _l);
                return std::make_pair(m, M);
            }
        };

        /** Logarithmic (base 10) scale. 
         * 
         * @warning This implementation is not very robust to rounding errors
         *          especially if you use the bounds method with large lengths.
         * 
         * @ingroup EAH
         */
        template <class R>
        class Log10Scale : public Scale<R> {
        protected:
            //! The minimum.
            const double _m;
            //! Max - min.
            const double _l;
            //! The number of buckets.
            const double _s;
            //! The `log(1+max-min)` constant.
            const double _k;

        public:
            /** Constructor.
             * 
             * @param min the minimum of the scale domain.
             * @param max the maximum of the scale domain.
             * @param size the number of buckets in the scale.
             */
            Log10Scale(const R min, const R max, const size_t size)
                : Scale<R>(min, max, size),
                _m(min),
                _l(max-min),
                _s(size),
                _k( std::log10(1+max-min) )
            {
                // FIXME check size against max-min ?
            }

            /** Returns the index of the bucket in which a value falls.
             *
             * @param x the value on the scale.
             * @returns the corresponding bucket index.
             */
            size_t index(const double x) const override
            {
                assert(this->_min <= x);
                assert(x <= this->max()); // FIXME do we want that?
                if(x >= this->max()) {
                    return this->size() - 1;
                } else {
                    return static_cast<size_t>(
                        std::floor( std::log10(1 + (x - _m)) / _k * _s )
                    );
                }
            }

            //! The type in which bounds are stored by the `bounds` method.
            using BoundsType = typename Scale<R>::BoundsType;

            /** Returns the bounds of the corresponding bucket index.
             *
             * @param i the index of the bucket.
             * @returns [min,max[ of the bucket i.
             */
            BoundsType bounds(const size_t i) const override
            {
                // FIXME this renders indices at more than 4 ULPs for doubles, find a more robust computation?
                assert(i < _s);
                const double m = std::pow(10.0, static_cast<double>(i)        * std::log10(1.0 + _l) / _s) - 1.0 + _m;
                const double M = std::pow(10.0,(static_cast<double>(i) + 1.0) * std::log10(1.0 + _l) / _s) - 1.0 + _m;
                assert(m < M);
                assert(0 < M-m);
                assert(M-m <= this->length());
                return std::make_pair(m, M);
            }
        };

        /** Type used to store a single run attainment bi-dimensional function.
         *
         * First dimension is error targets,
         * second dimension function evaluations targets.
         *
         * @note One expect to have a lot of those matrix in a real-life setting,
         * and the more general case is to have objective-function bounded computation times.
         * It is thus chosen to reduce the memory footprint, using the infamous bool vector specialization
         * instead of the faster vector of char.
         * During our tests, using char was 1.02 times faster, while consuming 1.4 more memory.
         * 
         * @ingroup EAH_API
         */
        using AttainmentMatrix = std::vector< // error targets
            std::vector< // function evaluations
                bool>>; // occurence count

        /** Pretty print an AttainmentMatrix.
         *
         * @ingroup EAH
         */
        inline std::ostream& operator<<(std::ostream& out,
            const AttainmentMatrix& mat)
        {
            size_t n_digits = 1;
            if (mat.back().back())
                n_digits = 1 + static_cast<size_t>(std::floor(std::log10(static_cast<double>(mat.back().back()))));

            out.precision(n_digits);
            for (const auto& i : mat) {
                for (const auto j : i)
                    out << std::fixed << j << " ";
                out << std::endl;
            }
            return out;
        }

        /** Type used to store all bi-dimensional attainment functions.
         *
         * First  dimension is the problem id,
         * second dimension is the dimension id,
         * third  dimension is the instance id.
         * fourth dimension is the run id.
         * Every item is an AttainmentMatrix.
         * 
         * @ingroup EAH_API
         */
        using AttainmentSuite = std::map<size_t, // problem // FIXME add a Suite level.
            std::map<size_t, // dim
                std::map<size_t, // instance
                    std::map<size_t, // runs
                        AttainmentMatrix>>>>;

    } // eah

    /** A logger that stores bi-dimensional error/evaluations discretized attainment matrices.
     *
     * A matrix is stored for each triplet (problem,dimension,instance),
     * everything being identified by its index.
     * Attainment matrices are boolean 2D tables, in which a 1 indicates that
     * an error target (row index) have been attained at an evaluation target (column index).
     *
     * The whole set of matrices can be accessed by the `get` accessor.
     * A single one can be accessed by the `at` accessor.
     * Scales can be get back by `*_range` accessors,
     * which give access to the corresponding `min` and `max` functions (@see range).
     *
     * @code
        using namespace ioh::problem;
        using namespace ioh::logger;
        unsigned int sample_size = 100;
        unsigned int buckets = 20;
        ioh::suite::BBOB suite({1, 2}, {1, 2}, {2, 10});
        ioh::logger::EAH logger(0, 6e7, buckets, 0, sample_size, buckets);
        suite.attach_logger(logger);
        for (const auto &p : suite) {
            for (auto r = 0; r < 2; r++) {
                for (auto s = 0; s < sample_size; ++s) {
                    (*p)(ioh::common::Random::uniform(p->meta_data().n_variables));
                } // s in sample_size
                p->reset(); // New run
            } // r in runs
        } // p in suite
     * @endcode
     *
     * @note If you use as many buckets as there is evaluations of the objective function,
     *       you will essentially computes as many ERT-EAH as there is targets.
     * 
     * @ingroup Loggers
     * @ingroup EAH
     */
    class EAH : public Logger {
    protected:
        /** Internal types  @{ */
        /** Keep essential metadata about the problem. */
        struct Problem { // FIXME add the suite
            int pb {};
            int dim {};
            int ins {};
            int run {};
            bool has_opt {};
            bool is_tracked {};
            double opt;
            common::OptimizationType max_min {};
        };

        /** @} */

    public:
        /** Simple constructor that defaults to log-log scale.
         */
        EAH(
            const double error_min, const double error_max,
            const size_t error_buckets,
            const size_t evals_min, const size_t evals_max,
            const size_t evals_buckets)
            : Logger()
            , _default_range_error(error_min, error_max, error_buckets)
            , _default_range_evals(evals_min, evals_max, evals_buckets)
            , _range_error(_default_range_error)
            , _range_evals(_default_range_evals)
            , _empty(error_buckets,
                  std::vector<bool>(evals_buckets,
                      false))
        {
            // Insert references after members are instantiated.
            _triggers.insert(std::ref(_on_improvement));
            assert(_triggers.size() > 0);
            _properties.insert_or_assign(_transformed_y_best.name(), _transformed_y_best);
            _properties.insert_or_assign(_evaluations.name(), _evaluations);
            assert(consistent_properties());
        }

        /** Complete constructor, with which you can define linear or semi-log scale.
         *
         * @see ScaleLinear
         */
        EAH(
            eah::Scale<double>& error_buckets,
            eah::Scale<size_t>& evals_buckets)
            : Logger()
            , _default_range_error(0, 1, 1)
            , _default_range_evals(0, 1, 1)
            , _range_error(error_buckets)
            , _range_evals(evals_buckets)
            , _empty(error_buckets.size(),
                  std::vector<bool>(evals_buckets.size(),
                      false))
        {
            // Insert references after members are instantiated.
            _triggers.insert(std::ref(_on_improvement));
            assert(_triggers.size() > 0);
            _properties.insert_or_assign(_transformed_y_best.name(), _transformed_y_best);
            _properties.insert_or_assign(_evaluations.name(), _evaluations);
            assert(consistent_properties());
        }

    public:
        /** Logger interface  @{ */
        //! Not used, but part of the interface.
        void attach_suite(const std::string&) override // FIXME this should ADD a Suite, not reset.
        {
            clear();
        }

        //! Initialize on the given problem.
        void attach_problem(const problem::MetaData& problem) override
        {
            Logger::attach_problem(problem);
            _current.pb = problem.problem_id;
            _current.dim = problem.n_variables;
            _current.ins = problem.instance;
            _current.max_min = problem.optimization_type;
            _current.is_tracked = false;
            _current.run = 1 + static_cast<int>(_eah_suite[static_cast<int>(_current.pb)][static_cast<int>(_current.dim)][static_cast<int>(_current.ins)].size());
        }

        /** Actually store information about the last evaluation.
         */
        void call(const logger::Info& log_info) override
        {
            if (!_current.is_tracked) {
                _current.is_tracked = true;
                _current.has_opt = (log_info.optimum.y != std::numeric_limits<double>::infinity() and log_info.optimum.y != -std::numeric_limits<double>::infinity());
                if (_current.has_opt) {
                    IOH_DBG(info, "Problem has known optimum, will compute the EAH of the error.");
                    _current.opt = log_info.optimum.y;
                } else {
                    IOH_DBG(info, "Problem has no known optimum, will compute the absolute EAH.");
                }
                init_eah(_current);
            }

            // Access the properties that were instantiated in the constructor.
            const std::optional<double> transformed_y_best = _properties.at("transformed_y_best").get()(log_info);
            assert(transformed_y_best); // Assert that the optional holds a value, which should be the case here.
            const std::optional<double> evaluations        = _properties.at("evaluations").get()(log_info);
            assert(evaluations);

            double err;
            if (_current.has_opt) {
                err = std::abs(_current.opt - transformed_y_best.value());
            } else {
                err = transformed_y_best.value();
            }

            // If this target is worst than the domain.
            if (evaluations.value() < _range_evals.min() || _range_evals.max() < evaluations.value()
                || err < _range_error.min() || _range_error.max() < err) {
                // Discard it.
                // FIXME we should use a more generic debug log system
                std::clog << "WARNING: target out of domain. "
                          << "value:" << err << " [" << _range_error.min() << ","
                          << _range_error.max() << "], "
                          << "evals:" << evaluations.value() << " [" << _range_evals.min() << "," << _range_evals.max() << "]"
                          << " This measure is discarded!"
                          << std::endl;
                return;
            }

            // If the target is in domain
            assert(_range_error.min() <= err && err <= _range_error.max());
            const auto i_error = _range_error.index(err);

            assert(
                _range_evals.min() <= evaluations.value() && evaluations.value() <= _range_evals.max());
            const auto j_evals = _range_evals.index(static_cast<double>(evaluations.value()));

            // Fill up the dominated quadrant of the attainment matrix with ones
            // (either the upper/upper or lower/lower, depending on if it's
            // a min or max problem and on if the optimum is known).
            fill_up(i_error, j_evals);
        }

        /** @} */ // Logger Interface

    public:
        /** Accessors @{ */

        /** Access all the data computed by this observer. */
        [[nodiscard]] const eah::AttainmentSuite& data() const
        {
            return _eah_suite;
        }

        /** Access a single attainment matrix.
         *
         * @note Use the same indices order than problem.
         *
         * First index: problem id,
         * second index: instance id,
         * third index: dimension id.
         * last index: run id.
         */
        [[nodiscard]] const eah::AttainmentMatrix& at(size_t problem_id, size_t instance_id,
            size_t dim_id, size_t runs) const
        {
            assert(_eah_suite.count(problem_id) != 0);
            assert(_eah_suite.at(problem_id).count(dim_id) != 0);
            assert(
                _eah_suite.at(problem_id).at(dim_id).count(instance_id) != 0);
            assert(
                _eah_suite.at(problem_id).at(dim_id).at(instance_id).count(runs) != 0);
            return _eah_suite.at(problem_id).at(dim_id).at(instance_id).at(runs);
        }

        /** Returns the size of the computed data, in its internal order.
         *
         * @note: the order of the indices is not the one used by logger interface!
         *
         * First index: number of problems,
         * second index: number of dimensions,
         * third index: number of instances.
         * last index: number of runs.
         */
        std::tuple<size_t, size_t, size_t, size_t> size()
        {
            return std::make_tuple(
                _eah_suite.size(), // problems
                _eah_suite.begin()->second.size(), // dimensions
                _eah_suite.begin()->second.begin()->second.size(),
                // instances
                _eah_suite.begin()->second.begin()->second.begin()->second.size() // runs
            );
        }

        /** Accessor to the range used for error targets in this logger instance. */
        [[nodiscard]] eah::Scale<double>& error_range() const
        {
            return _range_error;
        }

        /** Accessor to the range used for evaluations targets in this logger instance. */
        [[nodiscard]] eah::Scale<size_t>& eval_range() const
        {
            return _range_evals;
        }

        /** @} */

    protected:
        /** Internal methods  @{ */

        //! Clear all previously computed data structure.
        void clear()
        {
            _eah_suite.clear();
        }

        //! Create maps and matrix for this problem.
        void init_eah(const Problem& cur)
        {
            if (_eah_suite.count(cur.pb) == 0) {
                _eah_suite[cur.pb] = std::map<
                    size_t, std::map<size_t, std::map<size_t, eah::AttainmentMatrix>>>();
            }
            if (_eah_suite[cur.pb].count(cur.dim) == 0) {
                _eah_suite[cur.pb][cur.dim] = std::map<
                    size_t, std::map<size_t, eah::AttainmentMatrix>>();
            }
            if (_eah_suite[cur.pb][cur.dim].count(cur.ins) == 0) {
                _eah_suite[cur.pb][cur.dim][cur.ins] = std::map<
                    size_t, eah::AttainmentMatrix>();
            }
            _eah_suite[cur.pb][cur.dim][cur.ins][cur.run] = _empty;

            assert(
                _eah_suite.at(cur.pb).at(cur.dim).at(cur.ins).at(cur.run).at(0).at(0)
                == 0);
        }

        //! Returns the current attainment matrix.
        eah::AttainmentMatrix& current_eah()
        {
            assert(_eah_suite.count(_current.pb) != 0);
            assert(_eah_suite[_current.pb].count(_current.dim) != 0);
            assert(
                _eah_suite[_current.pb][_current.dim].count(_current.ins)
                != 0);
            assert(
                _eah_suite[_current.pb][_current.dim][_current.ins].count(_current.run)
                != 0);
            return _eah_suite[_current.pb][_current.dim][_current.ins][_current.run];
        }

        /** Fill up the upper/upper quadrant of the attainment matrix with ones.
         *
         * Take care of not losing time overwriting existing quadrants.
         */
        void fill_up(size_t i_error, size_t j_evals)
        {
            auto& mat = current_eah();
            const auto ibound = _range_error.size();
            auto jbound = _range_evals.size();

            if (_current.has_opt || _current.max_min == common::OptimizationType::Minimization) {
                for (auto i = i_error; i < ibound; i++) {
                    // If we reach a 1 on first col of this row, no need to continue.
                    // TODO: check this, out of bound error for mscv
                    if (mat[i][std::min(j_evals, jbound - 1)] == 1) {
                        break;
                    }
                    for (auto j = j_evals; j < jbound; j++) {
                        // If we reach a 1 on this col, no need to fill
                        // the remaining columns for the next rows..
                        if (mat[i][j] == 1) {
                            jbound = j;
                            break;
                        }
                        mat[i][j] = true;
                    } // for j
                } // for i
            } else {
                assert(
                    !_current.has_opt && _current.max_min == common::OptimizationType::Maximization);
                for (auto i = i_error; i >= 1; i--) {
                    // If we reach a 1 on first col of this row, no need to continue.
                    if (mat[i - 1][j_evals] == 1) {
                        continue;
                    }
                    for (auto j = j_evals; j < jbound; j++) {
                        // If we reach a 1 on this col, no need to fill
                        // the remaining columns for the next rows..
                        if (mat[i - 1][j] == 1) {
                            jbound = j;
                            break;
                        }
                        mat[i - 1][j] = true;
                    } // for j
                } // for i
            }
        }

        /** @} */

    private:
        //! Default range for errors is logarithmic.
        eah::Log10Scale<double> _default_range_error;

        //! Default range for evaluations is logarithmic.
        eah::Log10Scale<size_t> _default_range_evals;

    protected:
        /** Internal members  @{ */

        //! Scale for the errors axis.
        eah::Scale<double>& _range_error;

        //! Scale for the evaluations axis.
        eah::Scale<size_t>& _range_evals;

        //! Currently targeted problem metadata.
        Problem _current;

        //! An attainment matrix filled with zeros, copied for each new problem/instance/dim.
        eah::AttainmentMatrix _empty;

        //! The whole main data structure.
        eah::AttainmentSuite _eah_suite;

        /** Default trigger is on every improvement.
        *
        * Because it fits the algorithmics.
        */
        trigger::OnImprovement _on_improvement;

        //! Property watching the number of evaluations.
        watch::Evaluations _evaluations;

        //! Property watching the objective function value.
        watch::TransformedYBest _transformed_y_best;

        /** @} */
    };


    namespace eah {

        /** An interface for classes that computes statistics
         * over the AttainmentSuite computed by an eah::EAH logger.
         *
         * The template indicates the return type of the functor interface.
         * 
         * @ingroup EAH_API
         */
        template <class T>
        class Stat {
        public:
            //! Type of the computed statistic.
            using Type = T;

            /** Call interface. */
            virtual T operator()(const EAH& logger) = 0;
        };

        /** Various statistics which can be readily applied on the data gathered by a logger::EAH. */
        namespace stat {

            /** Generic functor for accumulated statistics.
             * 
             * Most probably called from a function defaulting the basic operation, like stat::sum,
             * or used in a function which compute something else.
             * 
             * @ingroup EAH_API
             */
            template<class T, class BinaryOperation>
            T accumulate(const EAH& logger, const T init, const BinaryOperation& op)
            {
                const AttainmentSuite& attainment = logger.data();
                assert(attainment.size() > 0);

                T res = init;
                for (const auto& pb_dim : attainment) {
                    assert(pb_dim.second.size() > 0);
                    for (const auto& dim_ins : pb_dim.second) {
                        assert(dim_ins.second.size() > 0);
                        for (const auto& ins_run : dim_ins.second) {
                            assert(ins_run.second.size() > 0);
                            for (const auto& run_att : ins_run.second) {
                                const auto& mat = run_att.second;
                                assert(mat.size() > 0);
                                assert(mat[0].size() > 0);
                                for (const auto& row : mat) {
                                    assert(row.size() > 0);
                                    for (const auto& item : row) {
                                        res = op(res, item);
                                    } // item
                                } // row
                            } // run_att
                        } // ins_run
                    } // dim_nis
                } // pb_dim
                return res;
            }

            /** Computes the sum of all elements of an AttainmentSuite data structure.
             *
             * @code
                    using namespace ioh::logger;
                    size_t s = eah::stat::sum(logger);
             * @endcode
             *
             * @ingroup EAH
             */
            inline size_t sum(const EAH& logger)
            {
                return accumulate<size_t>(logger, 0, std::plus<size_t>());
            }

            /** Computes the matrix that is the sum of all matrices in an AttainmentSuite.
             *  Across problems, dimensions and instances.
             * 
             * @note This functor holds the number of attainment matrix it hase encountered during its computation.
             *       If you just need the histogram, see the stat::histogram function.
             *
             * @code
                using namespcae ioh::logger;
                eah::stat::Histogram h;
                eah::stat::Histogram::Mat a = h(logger);
                size_t n = h.nb_attainments();
             * @endcode
             *
             * @ingroup EAH
             */
            class Histogram : public Stat<std::vector<std::vector<size_t>>> {
            protected:
                //! State check flag.
                bool _has_computed;
                //! Number of encountered attainment matrices.
                size_t _nb_att;

            public:
                /** The type used to store the histogram matrix.
                 * 
                 * @note Dimensions follow the AttainmentMatrix convention,
                 *       with errors as first dimension, and evaluations as
                 *       second dimension.
                */
                using Mat = std::vector<std::vector<size_t>>;

                //! Constructor.
                Histogram()
                : _has_computed(false)
                , _nb_att(0)
                { }

                /** Computes the histogram on the logger's data.
                 * 
                 * @param logger The logger::EAH.
                 * @returns a (\<nb of targets buckets\> * \<nb of evaluations buckets\>) matrix of positive integers.
                 */
                Mat operator()(const EAH& logger) override
                {
                    const eah::AttainmentSuite& attainment = logger.data();
                    assert(attainment.size() > 0);

                    const auto& a_pb0 = std::begin(attainment)->second;
                    assert(a_pb0.size() > 0);
                    const auto& a_dim0 = std::begin(a_pb0)->second;
                    assert(a_dim0.size() > 0);
                    const auto& a_ins0 = std::begin(a_dim0)->second;
                    assert(a_ins0.size() > 0);
                    const auto& a_run0 = std::begin(a_ins0)->second;
                    assert(a_run0.size() > 0);

                    // Infer size from the first item.
                    Mat agg(a_run0.size(), std::vector<size_t>(a_run0.at(0).size(), 0));

                    _nb_att = 0;
                    for (const auto& pb_dim : attainment) { // problem -> dimension map
                        assert(pb_dim.second.size() > 0);
                        for (const auto& dim_ins : pb_dim.second) { // dimension -> instance map
                            assert(dim_ins.second.size() > 0);
                            for (const auto& ins_run : dim_ins.second) { // instance -> run map
                                assert(ins_run.second.size() > 0);
                                for (const auto& run_att : ins_run.second) { // run -> attainment map

                                    const eah::AttainmentMatrix& mat = run_att.second;
                                    assert(mat.size() > 0);
                                    assert(mat[0].size() > 0);
                                    _nb_att++;

                                    assert(mat.size() == agg.size());
                                    assert(mat[0].size() == agg[0].size());
                                    for (size_t i = 0; i < mat.size(); ++i) {
                                        for (size_t j = 0; j < mat[0].size(); ++j) {
                                            agg[i][j] += mat[i][j];
                                        } // j
                                    } // i
                                } // run_att
                            } // ins_run
                        } // dim_ins
                    } // pb_dim

                    _has_computed = true;
                    return agg;
                }

                /** Returns the number of attainment matrices encountered during main computations.
                 * 
                 * @warning Will fail on a Debug assert if operator() has not been called at least once before.
                 * 
                 * @note Will not fail if several computations are called and you inadvertantly called it on the previous one.
                 */
                size_t nb_attainments() const
                {
                    assert(_has_computed);
                    return _nb_att;
                }
            };

            /** Computes the matrix that is the sum of all matrices in an AttainmentSuite.
             *  Across problems, dimensions and instances.
             * 
             * That is, a (\<nb of targets buckets\> * \<nb of evaluations buckets\>) matrix of positive integers.
             * 
             * Useful if you want to display the related histogram, for instance.
             * 
             * @note You can get the return type from Histogram::Mat
             *       (normally a simple vector<vector<size_t>>).
             * @code
                    using namespace ioh::logger;
                    eah::stat::Histogram::Mat m = eah::stat::histogram(logger);
             * @endcode
             *
             * @ingroup EAH
             */
            inline Histogram::Mat histogram(const EAH& logger)
            {
                Histogram h;
                return h(logger);
            }

            namespace Distribution { // Naming scheme consistent with Histogram.
                /** The type used to store the distribution matrix.
                 * 
                 * @note Dimensions follow the AttainmentMatrix convention,
                 *       with errors as first dimension, and evaluations as
                 *       second dimension.
                 */
                using Mat = std::vector<std::vector<double>>;
            }

            /** Computes the matrix that is joint empirical cumulative distribution function of all attainments in a logger.
             *  Across problems, dimensions and instances.
             * 
             * That is, a (\<nb of targets buckets\> * \<nb of evaluations buckets\>) matrix of probabilities.
             * 
             * This is actually the Histogram / nb_attaiments.
             * 
             * @note You can get the return type from Distribution::Mat
             *       (normally a simple vector<vector<double>>).
             * @code
                    using namespace ioh::logger;
                    eah::stat::Distribution::Mat m = eah::stat::distribution(logger);
             * @endcode
             *
             * @ingroup EAH
             */
            inline std::vector<std::vector<double>> distribution(const EAH& logger)
            {
                Histogram histo;
                Histogram::Mat h = histo(logger);
                const double n = histo.nb_attainments();

                Distribution::Mat res; res.reserve(h.size());

                for(const auto& hl : h) {
                    std::vector<double> rl; rl.reserve(hl.size());
                    for(const auto& i : hl) {
                        rl.push_back(static_cast<double>(i)/n);
                    }
                    res.push_back(rl);
                }

                return res;
            }

            /** Statistics over the normalized attainment space. */
            namespace under_curve {

                /** Generic function for accumulated statistics over the normalized attainment space.
                 * 
                 * Most probably used from a function defaulting the basic operation, like under_curve::volume,
                 * or used as a function to compute something else.
                 *
                 * @note The operation is computed on normalized dimensions (i.e. error, evals and probas),
                 *       so as to avoid a scale bias in favor of one (or the other) dimension.
                 * 
                 * You most probably only need the simplified interface:
                 * @code
                        using namespace ioh::logger;
                        EAH logger(0,1,2,3,4,5); // Whatever
                        // This is the definition of under_curve::volume, for instance:
                        double v = eah::stat::under_curve::accumulate(logger, 0.0, std::plus<double>());
                 * @endcode
                 *
                 * @ingroup EAH_API
                 */
                template<class BinaryOperation>
                double accumulate(const EAH& logger, double init, BinaryOperation op)
                {
                    const eah::AttainmentSuite& attainment = logger.data();
                    assert(attainment.size() > 0);

                    Histogram histo;
                    Histogram::Mat mat = histo(logger);
                    assert(histo.nb_attainments() > 0);

                   const eah::Scale<double>& range_error = logger.error_range();
                   const eah::Scale<size_t>& range_evals = logger.eval_range();

                    double res = init;
                    for (size_t i = 0; i < mat.size(); ++i) {
                        for (size_t j = 0; j < mat[0].size(); ++j) {
                            double w_proba =  mat[i][j] / histo.nb_attainments();
                            assert(0 <= w_proba and w_proba <= 1);
                            // Within the loop because widths of buckets vary for log ranges.
                            const double w_error = (range_error.bounds(i).second - range_error.bounds(i).first) / range_error.length();
                            assert(0 <= w_error and w_error <= 1);
                            const double w_evals = (range_evals.bounds(j).second - range_evals.bounds(j).first) / range_evals.length();
                            assert(0 <= w_evals and w_evals <= 1);
                            // TODO allow to multiply by a weight each axis?
                            res = op(res, w_proba * w_error * w_evals);
                        } // j
                    } // i
                    return res;
                }

                /** Computes the normalized volume under the curve of the logger::EAH's data structure.
                 *
                 * @code
                        using namespace iof::logger;
                        double v = eah::stat::under_curve::volume(logger);
                 * @endcode
                 *
                 * @note This is just a proxy to the stat::under_curve::accumulate function,
                 *       which provides a more detailled interface.
                 *
                 * @ingroup EAH
                 */
                inline double volume(const EAH& logger)
                {
                    return accumulate(logger, 0.0, std::plus<double>());
                }

            } // under_curve
        } // stat


        /** Print a 2D colormap as a 256-color ASCII-art.
         * 
         * Convenience function to print the 2D vector arrays outputed by histogram or distribution.
         * 
         * Useful to rapidely check or debug the core data structure of the EAH logger.
         * 
         * By default, just display the colormap.
         * If ranges are passed, display axis legends with values.
         * 
         * @note The y-axis lowver buckets values are printed as vertical numbers.
         *       The x-axis legend show both min and max of buckets.
         * 
         * @ingroup EAH
         * 
         * @param data A 2D vector array.
         * @param ranges The pair of Scale used for the computation of data. If given, will add axis legends.
         * @param values If true, will display one 2-digits value over two within the colored pixels. Numbers with more than 2 digits are rendered as "++".
          */
        template<class T>
        std::string colormap(const std::vector<std::vector<T>>& data,
                            std::pair<const ioh::logger::eah::Scale<double> *, const ioh::logger::eah::Scale<size_t> *> ranges = {nullptr,nullptr},
                            bool values = false)
        {
            size_t fg_shift = 128;

            // Optional axis legends.
            const ioh::logger::eah::Scale<double> * range_error;
            const ioh::logger::eah::Scale<size_t> * range_evals;
            std::tie(range_error, range_evals) = ranges;
            bool has_ranges = (range_error and range_evals);

            size_t height = data.size();
            assert(height>0);
            size_t width = data.at(0).size();
            assert(width>0);

            // Min/max of values.
            double vmin = std::numeric_limits<T>::max();
            double vmax = std::numeric_limits<T>::min();
            for(std::size_t i=0; i < 0+height; ++i) {
                for(std::size_t j=0; j < 0+width; ++j) {
                    double val = data[i][j];
                    if( not std::isnan(val) and not std::isinf(val) ) {
                        vmin = std::min(val, vmin);
                        vmax = std::max(val, vmax);
                    }
                }
            }

            const double nb_colors = 255;
            std::ostringstream out;

            // Colormap legend
            if(has_ranges) {
                std::ostringstream legend;
                legend << "[" << vmin << "";
                for(size_t i = 0; i < 2*height; ++i) {
                    // RGB ANSI codes for greys.
                    size_t color = static_cast<size_t>(std::floor( static_cast<double>(i) / static_cast<double>(2*height) * nb_colors-1 ));
                    legend << "\033[48;2;" << color << ";" << color << ";" << color << "m" << " ";
                }
                legend << "\033[0m"; // Reset.
                legend << "" << vmax << "]";
                out << legend.str() << std::endl;
                out << std::string(2*width, ' ') << " Errors:" << std::endl;
            }

            // Pixel map
            std::string nan = "╳╳";
            for(size_t i=0; i < height; ++i) {
                for(size_t j=0; j < width; ++j) {
                    double val = data[i][j];
                    if( std::isnan(val) or std::isinf(val) ) {
                        out << nan;
                    } else {
                        size_t color = std::floor((val-vmin)/(vmax-vmin)*(nb_colors-1));

                        if(color >= nb_colors) color = nb_colors-1;

                        if( vmin <= val and val <= vmax ) {
                            // Value as background, white or black as foreground.
                            out << "\033[48;2;" << color << ";" << color << ";" << color;
                            size_t fg = (color-fg_shift) % static_cast<size_t>(nb_colors);
                            out << ";38;2;" << fg << ";" << fg << ";" << fg;
                            out << "m";

                            // Print one value over two, if asked.
                            if(values and j%2 == 0) {
                                std::ostringstream fmt;
                                fmt << val;
                                if(fmt.str().size() == 1) {
                                    out << " " << fmt.str();
                                } else if(fmt.str().size() == 2) {
                                    out << fmt.str();
                                } else {
                                    out << "++";
                                }
                            } else {
                                out << "  "; // Two spaces for a squared display.
                            }

                            out << "\033[0m"; // Colors reset.
                        } else {
                            out << nan;
                        }
                    } // if nan or inf
                } // j
                if(has_ranges) out << " [" << range_error->bounds(i).first << " .. " << range_error->bounds(i).second << "]";
                out << std::endl;
            } // i

            if(has_ranges) {
                out << "Evaluations:" << std::endl;
                size_t max_size = 0;
                std::vector<std::string> evals; evals.reserve(width);
                for(size_t j=0; j < width; ++j) {
                    std::ostringstream fmt;
                    fmt << range_evals->bounds(j).first;
                    max_size = std::max(max_size, fmt.str().size());
                    evals.push_back(fmt.str());
                }
                // Last bound
                std::ostringstream fmt;
                fmt << range_evals->bounds(width-1).second;
                max_size = std::max(max_size, fmt.str().size());
                evals.push_back(fmt.str());

                for(size_t i=0; i < max_size; ++i) { 
                    for(size_t j=0; j < width+1; ++j) {
                        if(i<evals[j].size()) out << evals[j][i] << " ";
                        else out << "  ";
                    }
                    out << std::endl;
                }
            }

            return out.str();
        }

    } // eah
} // logger
} // ioh
