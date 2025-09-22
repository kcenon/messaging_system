#!/bin/bash

# Network System Migration Quick Start Script
# 빠른 시작을 위한 마이그레이션 스크립트
#
# 작성자: kcenon
# 작성일: 2025-09-19

set -euo pipefail

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"

# 색상 코드
readonly GREEN='\033[0;32m'
readonly BLUE='\033[0;34m'
readonly YELLOW='\033[1;33m'
readonly RED='\033[0;31m'
readonly NC='\033[0m'

log_info() {
    echo -e "${BLUE}[INFO]${NC} $*"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $*"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $*"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $*"
}

show_header() {
    echo "================================================="
    echo "    Network System Migration Quick Start"
    echo "================================================="
    echo
}

show_menu() {
    echo "다음 중 원하는 작업을 선택하세요:"
    echo
    echo "1) 📋 마이그레이션 계획 검토"
    echo "2) 🔍 사전 조건 확인"
    echo "3) 🚀 전체 마이그레이션 실행"
    echo "4) 🧪 단계별 마이그레이션 실행"
    echo "5) ✅ 검증 및 테스트"
    echo "6) 📊 진행 상황 확인"
    echo "7) 🆘 도움말 및 문서"
    echo "0) 종료"
    echo
}

review_plan() {
    log_info "마이그레이션 계획 문서를 확인합니다..."
    echo

    if [[ -f "$PROJECT_ROOT/NETWORK_SYSTEM_SEPARATION_PLAN.md" ]]; then
        log_success "분리 계획서: $PROJECT_ROOT/NETWORK_SYSTEM_SEPARATION_PLAN.md"
    else
        log_warn "분리 계획서를 찾을 수 없습니다."
    fi

    if [[ -f "$PROJECT_ROOT/TECHNICAL_IMPLEMENTATION_DETAILS.md" ]]; then
        log_success "기술 구현 세부사항: $PROJECT_ROOT/TECHNICAL_IMPLEMENTATION_DETAILS.md"
    else
        log_warn "기술 구현 세부사항을 찾을 수 없습니다."
    fi

    if [[ -f "$PROJECT_ROOT/MIGRATION_CHECKLIST.md" ]]; then
        log_success "마이그레이션 체크리스트: $PROJECT_ROOT/MIGRATION_CHECKLIST.md"
    else
        log_warn "마이그레이션 체크리스트를 찾을 수 없습니다."
    fi

    echo
    read -p "문서를 열어보시겠습니까? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        if command -v code &> /dev/null; then
            code "$PROJECT_ROOT"/*.md
        elif command -v open &> /dev/null; then
            open "$PROJECT_ROOT"/*.md
        else
            log_info "기본 에디터로 문서를 여세요: $PROJECT_ROOT/"
        fi
    fi
}

check_prerequisites() {
    log_info "사전 조건을 확인합니다..."
    echo

    local all_good=true

    # 디렉토리 확인
    local sources_root="$(dirname "$PROJECT_ROOT")"
    local messaging_system="$sources_root/messaging_system"

    if [[ -d "$messaging_system" ]]; then
        log_success "messaging_system 발견: $messaging_system"
    else
        log_error "messaging_system을 찾을 수 없습니다: $messaging_system"
        all_good=false
    fi

    if [[ -d "$messaging_system/network" ]]; then
        log_success "messaging_system/network 발견"
        local file_count=$(find "$messaging_system/network" -name "*.cpp" -o -name "*.h" | wc -l)
        log_info "C++ 파일 개수: $file_count"
    else
        log_error "messaging_system/network을 찾을 수 없습니다"
        all_good=false
    fi

    # 도구 확인
    local tools=("cmake" "make" "rsync" "git")
    for tool in "${tools[@]}"; do
        if command -v "$tool" &> /dev/null; then
            log_success "$tool: $(which "$tool")"
        else
            log_error "$tool을 찾을 수 없습니다"
            all_good=false
        fi
    done

    # Git 상태 확인
    if [[ -d "$messaging_system/.git" ]]; then
        cd "$messaging_system"
        if git diff-index --quiet HEAD --; then
            log_success "messaging_system Git 상태: 깨끗함"
        else
            log_warn "messaging_system에 커밋되지 않은 변경사항이 있습니다"
        fi
    fi

    # vcpkg 확인
    if command -v vcpkg &> /dev/null; then
        log_success "vcpkg: $(which vcpkg)"
    else
        log_warn "vcpkg를 찾을 수 없습니다 (선택사항)"
    fi

    echo
    if [[ "$all_good" == true ]]; then
        log_success "모든 사전 조건이 충족되었습니다!"
        echo
        read -p "마이그레이션을 진행하시겠습니까? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            return 0
        fi
    else
        log_error "일부 사전 조건이 충족되지 않았습니다."
        echo "누락된 항목을 설치하고 다시 시도하세요."
    fi

    return 1
}

run_full_migration() {
    log_info "전체 마이그레이션을 실행합니다..."
    echo

    if [[ ! -f "$SCRIPT_DIR/migrate_network_system.sh" ]]; then
        log_error "마이그레이션 스크립트를 찾을 수 없습니다: $SCRIPT_DIR/migrate_network_system.sh"
        return 1
    fi

    log_warn "이 작업은 기존 파일을 변경할 수 있습니다."
    log_warn "계속하기 전에 백업이 생성됩니다."
    echo
    read -p "계속하시겠습니까? (y/N): " -n 1 -r
    echo

    if [[ $REPLY =~ ^[Yy]$ ]]; then
        log_info "마이그레이션 스크립트를 실행합니다..."
        "$SCRIPT_DIR/migrate_network_system.sh"
    else
        log_info "마이그레이션이 취소되었습니다."
    fi
}

run_step_by_step() {
    log_info "단계별 마이그레이션을 실행합니다..."
    echo

    echo "다음 단계 중 하나를 선택하세요:"
    echo
    echo "1) Phase 1: 준비 및 분석"
    echo "2) Phase 2: 코어 시스템 분리"
    echo "3) Phase 3: 통합 인터페이스 구현"
    echo "4) Phase 4: messaging_system 업데이트"
    echo "5) Phase 5: 검증 및 배포"
    echo
    read -p "단계를 선택하세요 (1-5): " -n 1 -r
    echo

    case $REPLY in
        1)
            log_info "Phase 1: 준비 및 분석 단계를 실행합니다..."
            # 여기에 Phase 1 관련 스크립트 호출
            ;;
        2)
            log_info "Phase 2: 코어 시스템 분리 단계를 실행합니다..."
            # 여기에 Phase 2 관련 스크립트 호출
            ;;
        3)
            log_info "Phase 3: 통합 인터페이스 구현 단계를 실행합니다..."
            # 여기에 Phase 3 관련 스크립트 호출
            ;;
        4)
            log_info "Phase 4: messaging_system 업데이트 단계를 실행합니다..."
            # 여기에 Phase 4 관련 스크립트 호출
            ;;
        5)
            log_info "Phase 5: 검증 및 배포 단계를 실행합니다..."
            # 여기에 Phase 5 관련 스크립트 호출
            ;;
        *)
            log_warn "잘못된 선택입니다."
            ;;
    esac
}

run_verification() {
    log_info "검증 및 테스트를 실행합니다..."
    echo

    local sources_root="$(dirname "$PROJECT_ROOT")"
    local new_network_system="$sources_root/network_system"

    if [[ ! -d "$new_network_system" ]]; then
        log_error "새로운 network_system을 찾을 수 없습니다: $new_network_system"
        return 1
    fi

    cd "$new_network_system"

    # 빌드 테스트
    log_info "빌드 테스트를 실행합니다..."
    if [[ -f "build.sh" ]]; then
        if ./build.sh --no-tests; then
            log_success "빌드 테스트 통과"
        else
            log_error "빌드 테스트 실패"
            return 1
        fi
    else
        log_warn "build.sh를 찾을 수 없습니다"
    fi

    # 테스트 실행
    log_info "단위 테스트를 실행합니다..."
    if [[ -f "build/bin/network_system_tests" ]]; then
        if ./build/bin/network_system_tests; then
            log_success "단위 테스트 통과"
        else
            log_error "단위 테스트 실패"
        fi
    else
        log_warn "테스트 실행 파일을 찾을 수 없습니다"
    fi

    # 샘플 실행
    log_info "샘플 애플리케이션을 테스트합니다..."
    if [[ -f "build/bin/basic_echo_sample" ]]; then
        log_info "기본 에코 샘플을 5초간 실행합니다..."
        timeout 5s ./build/bin/basic_echo_sample || true
        log_success "샘플 실행 완료"
    else
        log_warn "샘플 실행 파일을 찾을 수 없습니다"
    fi
}

check_progress() {
    log_info "진행 상황을 확인합니다..."
    echo

    if [[ -f "$PROJECT_ROOT/MIGRATION_CHECKLIST.md" ]]; then
        log_info "체크리스트 파일을 확인합니다..."

        # 완료된 항목 수 계산
        local total_items=$(grep -c "^- \[ \]" "$PROJECT_ROOT/MIGRATION_CHECKLIST.md" || true)
        local completed_items=$(grep -c "^- \[x\]" "$PROJECT_ROOT/MIGRATION_CHECKLIST.md" || true)

        if [[ $total_items -gt 0 ]]; then
            local progress=$((completed_items * 100 / total_items))
            log_info "전체 진행률: $completed_items/$total_items ($progress%)"
        else
            log_info "체크리스트 항목을 찾을 수 없습니다"
        fi

        # Phase별 진행 상황
        echo
        log_info "Phase별 상태:"
        local phases=("Phase 1: 준비 및 분석" "Phase 2: 코어 시스템 분리" "Phase 3: 통합 인터페이스 구현" "Phase 4: messaging_system 업데이트" "Phase 5: 검증 및 배포")

        for phase in "${phases[@]}"; do
            if grep -q "## 🎯 $phase" "$PROJECT_ROOT/MIGRATION_CHECKLIST.md"; then
                log_info "  ✓ $phase: 계획됨"
            fi
        done
    else
        log_warn "체크리스트 파일을 찾을 수 없습니다"
    fi

    # 실제 파일 상태 확인
    echo
    log_info "실제 파일 상태:"

    local sources_root="$(dirname "$PROJECT_ROOT")"
    local network_system="$sources_root/network_system"

    if [[ -d "$network_system/network_system_new" ]]; then
        log_success "  새로운 network_system 구조 생성됨"
    elif [[ -d "$network_system/include/network_system" ]]; then
        log_success "  network_system 마이그레이션 완료"
    else
        log_warn "  network_system 마이그레이션 미완료"
    fi

    if [[ -f "$network_system/build.sh" ]]; then
        log_success "  빌드 스크립트 생성됨"
    else
        log_warn "  빌드 스크립트 미생성"
    fi
}

show_help() {
    log_info "도움말 및 문서를 표시합니다..."
    echo

    echo "📚 주요 문서:"
    echo "  • $PROJECT_ROOT/NETWORK_SYSTEM_SEPARATION_PLAN.md"
    echo "  • $PROJECT_ROOT/TECHNICAL_IMPLEMENTATION_DETAILS.md"
    echo "  • $PROJECT_ROOT/MIGRATION_CHECKLIST.md"
    echo

    echo "🛠️ 주요 스크립트:"
    echo "  • $SCRIPT_DIR/migrate_network_system.sh (전체 마이그레이션)"
    echo "  • $0 (이 스크립트)"
    echo

    echo "📋 마이그레이션 단계:"
    echo "  1. 사전 조건 확인"
    echo "  2. 백업 생성"
    echo "  3. 코드 분리 및 재구성"
    echo "  4. 빌드 시스템 설정"
    echo "  5. 통합 인터페이스 구현"
    echo "  6. 테스트 및 검증"
    echo

    echo "❓ 문제 해결:"
    echo "  • 빌드 실패: dependency.sh 실행 후 다시 시도"
    echo "  • 의존성 문제: vcpkg 설치 확인"
    echo "  • 권한 문제: 스크립트 실행 권한 확인"
    echo

    echo "📞 연락처:"
    echo "  • 담당자: kcenon"
    echo "  • 이메일: kcenon@naver.com"
    echo
}

main() {
    show_header

    while true; do
        show_menu
        read -p "선택하세요 (0-7): " -n 1 -r
        echo
        echo

        case $REPLY in
            1)
                review_plan
                ;;
            2)
                check_prerequisites
                ;;
            3)
                run_full_migration
                ;;
            4)
                run_step_by_step
                ;;
            5)
                run_verification
                ;;
            6)
                check_progress
                ;;
            7)
                show_help
                ;;
            0)
                log_info "종료합니다."
                exit 0
                ;;
            *)
                log_warn "잘못된 선택입니다. 다시 선택해주세요."
                ;;
        esac

        echo
        read -p "계속하려면 Enter를 누르세요..."
        echo
    done
}

# 스크립트 실행
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi