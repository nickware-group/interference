/////////////////////////////////////////////////////////////////////////////
// Name:        bindings.cpp
// Purpose:     pybind11 Python bindings for Interference library
// Created:     2026
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <indk/neuralnet.h>
#include <indk/neuron.h>
#include <indk/system.h>
#include <indk/profiler.h>
#include <indk/types.h>
#include <indk/position.h>
#include <indk/error.h>
#include <indk/backends/native.h>
#include <indk/backends/native_multithread.h>
#include <indk/backends/opencl.h>
#include <indk/backends/vulkan.h>

#include <fstream>
#include <sstream>

namespace py = pybind11;

PYBIND11_MODULE(_indk, m) {
    m.doc() = "Python bindings for the Interference neural network library";

    // ---------------------------------------------------------------
    // Exception
    // ---------------------------------------------------------------
    py::register_exception<indk::Error>(m, "Error", PyExc_RuntimeError);

    // ---------------------------------------------------------------
    // Enums
    // ---------------------------------------------------------------
    py::enum_<indk::System::ComputeBackends>(m, "ComputeBackend")
        .value("NativeCPU",            indk::System::ComputeBackends::NativeCPU)
        .value("NativeCPUMultithread", indk::System::ComputeBackends::NativeCPUMultithread)
        .value("OpenCL",               indk::System::ComputeBackends::OpenCL)
        .value("Vulkan",               indk::System::ComputeBackends::Vulkan)
        .export_values();

    py::enum_<indk::PatternCompareFlags>(m, "CompareFlag")
        .value("Default",    indk::CompareDefault)
        .value("Normalized", indk::CompareNormalized)
        .export_values();

    py::enum_<indk::ScopeProcessingMethods>(m, "ProcessingMethod")
        .value("Min",     indk::ProcessMin)
        .value("Average", indk::ProcessAverage)
        .export_values();

    py::enum_<indk::Neuron::ProcessingModes>(m, "ProcessingMode")
        .value("Default",      indk::Neuron::ProcessingModeDefault)
        .value("AutoReset",    indk::Neuron::ProcessingModeAutoReset)
        .value("AutoRollback", indk::Neuron::ProcessingModeAutoRollback)
        .export_values();

    py::enum_<indk::Neuron::OutputModes>(m, "OutputMode")
        .value("Stream",     indk::Neuron::OutputModeStream)
        .value("Latch",      indk::Neuron::OutputModeLatch)
        .value("Predefined", indk::Neuron::OutputModePredefined)
        .export_values();

    py::enum_<indk::Profiler::EventFlags>(m, "ProfilerEvent")
        .value("Processed", indk::Profiler::EventProcessed)
        .value("Tick",      indk::Profiler::EventTick)
        .export_values();

    // ---------------------------------------------------------------
    // OutputValue
    // ---------------------------------------------------------------
    py::class_<indk::OutputValue>(m, "OutputValue")
        .def_readonly("value",  &indk::OutputValue::value)
        .def_readonly("source", &indk::OutputValue::source)
        .def_readonly("time",   &indk::OutputValue::time)
        .def("__repr__", [](const indk::OutputValue &ov) {
            std::ostringstream ss;
            ss << "OutputValue(value=" << ov.value
               << ", source='" << ov.source
               << "', time=" << ov.time << ")";
            return ss.str();
        });

    // ---------------------------------------------------------------
    // ComputeBackendsInfo
    // ---------------------------------------------------------------
    py::class_<indk::ComputeBackendsInfo>(m, "ComputeBackendInfo")
        .def_readonly("backend_id",      &indk::ComputeBackendsInfo::backend_id)
        .def_readonly("backend_name",    &indk::ComputeBackendsInfo::backend_name)
        .def_readonly("translator_name", &indk::ComputeBackendsInfo::translator_name)
        .def_readonly("ready",           &indk::ComputeBackendsInfo::ready)
        .def("__repr__", [](const indk::ComputeBackendsInfo &bi) {
            std::ostringstream ss;
            ss << "ComputeBackendInfo(id=" << bi.backend_id
               << ", name='" << bi.backend_name
               << "', ready=" << (bi.ready ? "True" : "False") << ")";
            return ss.str();
        });

    // ---------------------------------------------------------------
    // Position
    // ---------------------------------------------------------------
    py::class_<indk::Position>(m, "Position")
        .def(py::init<>())
        .def(py::init<unsigned int, unsigned int>(), py::arg("xm"), py::arg("dimensions"))
        .def(py::init<unsigned int, std::vector<float>>(), py::arg("xm"), py::arg("values"))
        .def("get_dimensions_count", &indk::Position::getDimensionsCount)
        .def("get_xm",              &indk::Position::getXm)
        .def("get_value",           &indk::Position::getPositionValue, py::arg("index"))
        .def("get_distance_from",   &indk::Position::getDistanceFrom, py::arg("other"))
        .def("__repr__", [](const indk::Position &p) {
            std::ostringstream ss;
            ss << "Position(xm=" << p.getXm() << ", dims=" << p.getDimensionsCount() << ", [";
            for (unsigned i = 0; i < p.getDimensionsCount(); i++) {
                if (i > 0) ss << ", ";
                ss << p.getPositionValue(i);
            }
            ss << "])";
            return ss.str();
        });

    // ---------------------------------------------------------------
    // Neuron
    // ---------------------------------------------------------------
    py::class_<indk::Neuron>(m, "Neuron")
        .def_property_readonly("name",             &indk::Neuron::getName)
        .def_property_readonly("xm",               &indk::Neuron::getXm)
        .def_property_readonly("dimensions_count",  &indk::Neuron::getDimensionsCount)
        .def_property_readonly("latency",           &indk::Neuron::getLatency)
        .def_property_readonly("entries_count",     &indk::Neuron::getEntriesCount)
        .def_property_readonly("synapses_count",    &indk::Neuron::getSynapsesCount)
        .def_property_readonly("receptors_count",   &indk::Neuron::getReceptorsCount)
        .def_property_readonly("is_learned",        &indk::Neuron::isLearned)
        .def_property_readonly("processing_mode",   &indk::Neuron::getProcessingMode)
        .def_property_readonly("output_mode",        &indk::Neuron::getOutputMode)
        .def("get_entries",     &indk::Neuron::getEntries)
        .def("get_link_output", &indk::Neuron::getLinkOutput)
        .def("set_lambda",          &indk::Neuron::setLambda,         py::arg("value"))
        .def("set_k1",              &indk::Neuron::setk1,             py::arg("value"))
        .def("set_k2",              &indk::Neuron::setk2,             py::arg("value"))
        .def("set_k3",              &indk::Neuron::setk3,             py::arg("value"))
        .def("set_processing_mode", &indk::Neuron::setProcessingMode, py::arg("mode"))
        .def("set_output_mode",     &indk::Neuron::setOutputMode,     py::arg("mode"))
        .def("__repr__", [](indk::Neuron &n) {
            return "Neuron('" + n.getName() + "', xm=" + std::to_string(n.getXm()) +
                   ", dims=" + std::to_string(n.getDimensionsCount()) + ")";
        });

    // ---------------------------------------------------------------
    // NeuralNet
    // ---------------------------------------------------------------
    py::class_<indk::NeuralNet>(m, "NeuralNet")
        .def(py::init<>())
        .def(py::init<const std::string &>(), py::arg("path"))

        // Structure
        .def("set_structure", py::overload_cast<const std::string &>(&indk::NeuralNet::setStructure),
             py::arg("json_string"))
        .def("do_load_structure", [](indk::NeuralNet &self, const std::string &path) {
            std::ifstream f(path);
            if (!f.is_open())
                throw std::runtime_error("Cannot open file: " + path);
            return self.setStructure(f);
        }, py::arg("path"))
        .def("get_structure", &indk::NeuralNet::getStructure,
             py::arg("minimized") = true)

        // Properties
        .def_property_readonly("name",                  &indk::NeuralNet::getName)
        .def_property_readonly("description",           &indk::NeuralNet::getDescription)
        .def_property_readonly("version",               &indk::NeuralNet::getVersion)
        .def_property_readonly("neuron_count",          &indk::NeuralNet::getNeuronCount)
        .def_property_readonly("total_parameter_count", &indk::NeuralNet::getTotalParameterCount)
        .def_property_readonly("model_size",            &indk::NeuralNet::getModelSize)
        .def_property_readonly("instance_count",        &indk::NeuralNet::getInstanceCount)
        .def_property_readonly("is_learned",            &indk::NeuralNet::isLearned)

        // Instances
        .def("do_create_instance", &indk::NeuralNet::doCreateInstance,
             py::arg("backend") = (int)indk::System::ComputeBackends::NativeCPU)
        .def("do_create_instances", &indk::NeuralNet::doCreateInstances,
             py::arg("count"), py::arg("backend") = (int)indk::System::ComputeBackends::NativeCPU)
        .def("do_translate_to_instance", &indk::NeuralNet::doTranslateToInstance,
             py::arg("inputs") = std::vector<std::string>{}, py::arg("instance") = 0)

        // Learn / Recognise
        .def("do_learn", &indk::NeuralNet::doLearn,
             py::arg("data"),
             py::arg("reset") = true,
             py::arg("inputs") = std::vector<std::string>{},
             py::arg("instance") = 0,
             py::call_guard<py::gil_scoped_release>())
        .def("do_recognise", &indk::NeuralNet::doRecognise,
             py::arg("data"),
             py::arg("reset") = true,
             py::arg("inputs") = std::vector<std::string>{},
             py::arg("instance") = 0,
             py::call_guard<py::gil_scoped_release>())

        // Compare patterns (3 overloads)
        .def("do_compare_patterns",
             py::overload_cast<int, int, int>(&indk::NeuralNet::doComparePatterns),
             py::arg("compare_flag") = (int)indk::CompareDefault,
             py::arg("processing_method") = (int)indk::ProcessMin,
             py::arg("instance") = 0,
             py::call_guard<py::gil_scoped_release>())
        .def("do_compare_patterns_ensemble",
             py::overload_cast<const std::string&, int, int, int>(&indk::NeuralNet::doComparePatterns),
             py::arg("ensemble"),
             py::arg("compare_flag") = (int)indk::CompareDefault,
             py::arg("processing_method") = (int)indk::ProcessMin,
             py::arg("instance") = 0,
             py::call_guard<py::gil_scoped_release>())
        .def("do_compare_patterns_neurons",
             py::overload_cast<std::vector<std::string>, int, int, int>(&indk::NeuralNet::doComparePatterns),
             py::arg("neurons"),
             py::arg("compare_flag") = (int)indk::CompareDefault,
             py::arg("processing_method") = (int)indk::ProcessMin,
             py::arg("instance") = 0,
             py::call_guard<py::gil_scoped_release>())

        // Scope
        .def("do_create_new_scope", &indk::NeuralNet::doCreateNewScope)
        .def("do_change_scope",     &indk::NeuralNet::doChangeScope, py::arg("scope_id"))

        // Topology
        .def("do_add_output",                &indk::NeuralNet::doAddNewOutput, py::arg("name"))
        .def("do_include_neuron_to_ensemble", &indk::NeuralNet::doIncludeNeuronToEnsemble,
             py::arg("neuron"), py::arg("ensemble"))
        .def("do_replicate_neuron", &indk::NeuralNet::doReplicateNeuron,
             py::arg("from_name"), py::arg("to_name"), py::arg("integrate"),
             py::return_value_policy::reference_internal)
        .def("do_delete_neuron",      &indk::NeuralNet::doDeleteNeuron, py::arg("name"))
        .def("do_replicate_ensemble", &indk::NeuralNet::doReplicateEnsemble,
             py::arg("from_name"), py::arg("to_name"), py::arg("copy_entries") = false)

        // Reset
        .def("do_reset",
             py::overload_cast<int>(&indk::NeuralNet::doReset),
             py::arg("instance"))
        .def("do_reset_ensemble",
             py::overload_cast<const std::string &, int>(&indk::NeuralNet::doReset),
             py::arg("ensemble"), py::arg("instance"))
        .def("do_reset_neurons",
             py::overload_cast<const std::vector<std::string> &, int>(&indk::NeuralNet::doReset),
             py::arg("neurons"), py::arg("instance"))

        // State sync
        .def("set_state_sync_enabled", &indk::NeuralNet::setStateSyncEnabled,
             py::arg("enabled") = true)

        // Neuron access
        .def("get_neuron", &indk::NeuralNet::getNeuron, py::arg("name"),
             py::return_value_policy::reference_internal)
        .def("get_neurons", &indk::NeuralNet::getNeurons,
             py::return_value_policy::reference_internal)
        .def("get_ensemble", &indk::NeuralNet::getEnsemble, py::arg("name"),
             py::return_value_policy::reference_internal)
        .def("get_output_values", &indk::NeuralNet::getOutputValues,
             py::arg("neurons") = std::vector<std::string>{}, py::arg("instance") = 0)

        // Interlink
        .def("do_interlink_init", &indk::NeuralNet::doInterlinkInit,
             py::arg("port") = 4408, py::arg("timeout") = 5)
        .def("do_interlink_web_init", &indk::NeuralNet::doInterlinkWebInit,
             py::arg("path"), py::arg("port") = 8044)

        .def("__repr__", [](indk::NeuralNet &nn) {
            return "NeuralNet('" + nn.getName() + "', neurons=" +
                   std::to_string(nn.getNeuronCount()) + ", params=" +
                   std::to_string(nn.getTotalParameterCount()) + ")";
        });

    // ---------------------------------------------------------------
    // System (module-level functions)
    // ---------------------------------------------------------------
    m.def("get_compute_backends_info", &indk::System::getComputeBackendsInfo,
          "Get information about all registered compute backends");

    m.def("is_compute_backend_available", &indk::System::isComputeBackendAvailable,
          py::arg("backend_id"),
          "Check if a compute backend is available");

    m.def("set_verbosity_level", &indk::System::setVerbosityLevel,
          py::arg("level"),
          "Set library verbosity level");

    m.def("get_verbosity_level", &indk::System::getVerbosityLevel,
          "Get current library verbosity level");

    // Backend parameters helpers
    m.def("set_cpu_multithread_workers", [](int worker_count) {
        indk::ComputeBackends::NativeCPUMultithread::Parameters params;
        params.worker_count = worker_count;
        indk::System::setComputeBackendParameters(
            indk::System::ComputeBackends::NativeCPUMultithread, &params);
    }, py::arg("worker_count"),
       "Set the number of worker threads for the NativeCPUMultithread backend");

    m.def("set_opencl_device", [](const std::string &device_name) {
        indk::ComputeBackends::OpenCL::Parameters params;
        params.device_name = device_name;
        indk::System::setComputeBackendParameters(
            indk::System::ComputeBackends::OpenCL, &params);
    }, py::arg("device_name"),
       "Set the OpenCL device by name");

    m.def("set_vulkan_device", [](const std::string &device_name) {
        indk::ComputeBackends::Vulkan::Parameters params;
        params.device_name = device_name;
        indk::System::setComputeBackendParameters(
            indk::System::ComputeBackends::Vulkan, &params);
    }, py::arg("device_name"),
       "Set the Vulkan device by name");

    // ---------------------------------------------------------------
    // Profiler
    // ---------------------------------------------------------------
    m.def("profiler_attach", [](indk::NeuralNet *net, int flag, py::function callback) {
        indk::Profiler::doAttachCallback(net, flag, [callback](indk::NeuralNet *nn) {
            py::gil_scoped_acquire acquire;
            callback(nn);
        });
    }, py::arg("net"), py::arg("event"), py::arg("callback"),
       "Attach a profiler callback to a neural net");

    // ---------------------------------------------------------------
    // Version info
    // ---------------------------------------------------------------
    m.attr("__version__") = "3.1.0";
}
