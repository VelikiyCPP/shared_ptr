#include <iostream>
#include <new>
#include <utility>

namespace my {
    template <typename T>
    class shared_ptr {
        struct control_block_base {
            std::size_t count_;

            explicit control_block_base(const std::size_t count = 1) noexcept : count_{count} {
                std::cout << "explicit control_block_base()" << std::endl;
            }

            virtual ~control_block_base() {
                std::cout << "~control_block_base()" << std::endl;
            }

            virtual void destroy_object() noexcept {} 
        };
        
        struct control_block : control_block_base {
            T object_;

            template <typename U>
            explicit control_block(U&& object) : control_block_base(1), object_(std::forward<U>(object)) {
                std::cout << "explicit control_block(U&& object) : object_(std::forward<U>(object))" << std::endl;
            }

            ~control_block() {
                std::cout << "~control_block()" << std::endl;
            }

            void destroy_object() noexcept {
                object_.~T();
            }
        };

        T* p_object_ = nullptr;
        control_block_base* p_control_block_ = nullptr; 
        
        template<typename U,typename... Args> 
        friend shared_ptr<U> make_shared(Args&&... args);

        explicit shared_ptr(control_block* p_control_block) noexcept : p_control_block_{p_control_block}, p_object_{&p_control_block->object_} {
            std::cout << "shared_ptr(control_block* p_control_block)" << std::endl;
        }

        void release() noexcept {
            if (p_control_block_) {
                p_control_block_->count_--;
                if (p_control_block_->count_ == 0) {
                    p_control_block_->destroy_object();
                    delete p_control_block_;
                    p_control_block_ = nullptr;
                    p_object_ = nullptr;
                }
            }
        }

    public:
        explicit shared_ptr(T* object) : p_object_{object} {
            if (object) {
                p_control_block_ = new control_block_base(1);
            }
            std::cout << "shared_ptr(T* object)" << std::endl;
        }

        shared_ptr() noexcept {
            std::cout << "shared_ptr()" << std::endl;
        }

        shared_ptr(const shared_ptr& other) noexcept : p_object_(other.p_object_), p_control_block_(other.p_control_block_) {
            if (p_control_block_) {
                p_control_block_->count_++;
            }
            std::cout << "shared_ptr copy constructor" << std::endl;
        }

        shared_ptr& operator=(const shared_ptr& other) noexcept {
            if (this != &other) {
                release();
                p_object_ = other.p_object_;
                p_control_block_ = other.p_control_block_;
                if (p_control_block_) {
                    p_control_block_->count_++;
                }
            }
            std::cout << "shared_ptr copy assignment" << std::endl;
            return *this;
        }

        ~shared_ptr() {
            release();
            std::cout << "~shared_ptr()" << std::endl;
        }

        T& operator*() const noexcept {
            return *p_object_;
        }

        T* operator->() const noexcept {
            return p_object_;
        }

        T* get() const noexcept {
            return p_object_;
        }

        std::size_t use_count() const noexcept {
            return p_control_block_ ? p_control_block_->count_ : 0;
        }

        bool unique() const noexcept {
            return use_count() == 1;
        }

        explicit operator bool() const noexcept {
            return p_object_ != nullptr;
        }
    };

    template<typename U, typename... Args>
    shared_ptr<U> make_shared(Args&&... args) {
        return shared_ptr<U>(new typename shared_ptr<U>::control_block(std::forward<Args>(args)...));
    }
}

class Car {
    public:
        int value;
        
        Car(int b) : value(b) {
            std::cout << "Car(" << b << ")" << std::endl;
        }

        ~Car() {
            std::cout << "~Car() value=" << value << std::endl;
        }
};

int main() {
    std::cout << "=== Test make_shared ===" << std::endl;
    my::shared_ptr<Car> ptr = my::make_shared<Car>(50);
    
    std::cout << "ptr->value = " << ptr->value << std::endl;
    std::cout << "(*ptr).value = " << (*ptr).value << std::endl;
    std::cout << "ptr.get()->value = " << ptr.get()->value << std::endl;
    std::cout << "use_count = " << ptr.use_count() << std::endl;
    std::cout << "is unique = " << ptr.unique() << std::endl;
    std::cout << "bool conversion = " << (ptr ? "true" : "false") << std::endl;

    return 0;
}
